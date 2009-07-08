// Filename: p3dSession.cxx
// Created by:  drose (03Jun09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "p3dSession.h"
#include "p3dInstance.h"
#include "p3dInstanceManager.h"
#include "p3d_plugin_config.h"
#include "p3dNullObject.h"
#include "p3dNoneObject.h"
#include "p3dBoolObject.h"
#include "p3dIntObject.h"
#include "p3dFloatObject.h"
#include "p3dPythonObject.h"

#ifndef _WIN32
#include <fcntl.h>
#endif

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::Constructor
//       Access: Public
//  Description: Creates a new session, corresponding to a new
//               subprocess with its own copy of Python.  The initial
//               parameters for the session are taken from the
//               indicated instance object (but the instance itself is
//               not automatically started within the session).
////////////////////////////////////////////////////////////////////
P3DSession::
P3DSession(P3DInstance *inst) {
  _session_key = inst->get_session_key();
  _python_version = inst->get_python_version();

  _p3dpython_running = false;
  _next_response_id = 0;
  _response = NULL;
  _got_response_id = -1;

  _started_read_thread = false;
  _read_thread_continue = false;

  _output_filename = inst->get_fparams().lookup_token("output_filename");

  _panda3d_callback = NULL;

  INIT_LOCK(_instances_lock);
  INIT_THREAD(_read_thread);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  _panda3d = inst_mgr->get_package("panda3d", "dev", "Panda3D");
  _python_root_dir = _panda3d->get_package_dir();
  inst->add_package(_panda3d);
}


////////////////////////////////////////////////////////////////////
//     Function: P3DSession::Destructor
//       Access: Public
//  Description: Terminates the session by shutting down Python and
//               stopping the subprocess.
////////////////////////////////////////////////////////////////////
P3DSession::
~P3DSession() {
  if (_panda3d_callback != NULL) {
    _panda3d->cancel_callback(_panda3d_callback);
    delete _panda3d_callback;
  }

  if (_p3dpython_running) {
    // Tell the process we're going away.
    TiXmlDocument doc;
    TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
    TiXmlElement *xcommand = new TiXmlElement("command");
    xcommand->SetAttribute("cmd", "exit");
    doc.LinkEndChild(decl);
    doc.LinkEndChild(xcommand);
    nout << "sent: " << doc << "\n" << flush;
    _pipe_write << doc << flush;

    // Also close the pipe, to help underscore the point.
    _pipe_write.close();

    // Closing _pipe_read before the thread has stopped can result in
    // a hang.  Don't need to close it yet.
    //  _pipe_read.close();

#ifdef _WIN32
    // Now give the process a chance to terminate itself cleanly.
    if (WaitForSingleObject(_p3dpython_handle, 2000) == WAIT_TIMEOUT) {
      // It didn't shut down cleanly, so kill it the hard way.
      nout << "Terminating process.\n" << flush;
      TerminateProcess(_p3dpython_handle, 2);
    }

    CloseHandle(_p3dpython_handle);

#else  // _WIN32

    // TODO: posix kill().

#endif  // _WIN32
  }

  // If there are any leftover commands in the queue (presumably
  // implying we have never started the python process), then delete
  // them now, unsent.
  Commands::iterator ci;
  for (ci = _commands.begin(); ci != _commands.end(); ++ci) {
    delete (*ci);
  }
  _commands.clear();

  join_read_thread();
  DESTROY_LOCK(_instances_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::start_instance
//       Access: Public
//  Description: Adds the indicated instance to the session, and
//               starts it running.  It is an error if the instance
//               has been started anywhere else.
//
//               The instance must have the same session_key as the
//               one that was passed to the P3DSession constructor.
////////////////////////////////////////////////////////////////////
void P3DSession::
start_instance(P3DInstance *inst) {
  assert(inst->_session == NULL);
  assert(inst->get_session_key() == _session_key);
  assert(inst->get_python_version() == _python_version);

  ACQUIRE_LOCK(_instances_lock);
  inst->_session = this;
  bool inserted = _instances.insert(Instances::value_type(inst->get_instance_id(), inst)).second;
  RELEASE_LOCK(_instances_lock);
  assert(inserted);

  TiXmlDocument *doc = new TiXmlDocument;
  TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "start_instance");
  TiXmlElement *xinstance = inst->make_xml();
  
  doc->LinkEndChild(decl);
  doc->LinkEndChild(xcommand);
  xcommand->LinkEndChild(xinstance);

  send_command(doc);
  inst->send_browser_script_object();

  if (_panda3d->get_ready()) {
    // If it's ready immediately, go ahead and start.
    start_p3dpython();
  } else {
    // Otherwise, set a callback, so we'll know when it is ready.
    if (_panda3d_callback == NULL) {
      _panda3d_callback = new PackageCallback(this);
      _panda3d->set_callback(_panda3d_callback);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::terminate_instance
//       Access: Public
//  Description: Removes the indicated instance from the session, and
//               stops it.  It is an error if the instance is not
//               already running on this session.
////////////////////////////////////////////////////////////////////
void P3DSession::
terminate_instance(P3DInstance *inst) {
  TiXmlDocument *doc = new TiXmlDocument;
  TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "terminate_instance");
  xcommand->SetAttribute("instance_id", inst->get_instance_id());
  
  doc->LinkEndChild(decl);
  doc->LinkEndChild(xcommand);

  send_command(doc);

  ACQUIRE_LOCK(_instances_lock);
  if (inst->_session == this) {
    inst->_session = NULL;
    _instances.erase(inst->get_instance_id());
  }
  RELEASE_LOCK(_instances_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::send_command
//       Access: Public
//  Description: Sends the indicated command to the running Python
//               process.  If the process has not yet been started,
//               queues it up until it is ready.
//
//               The command must be a newly-allocated TiXmlDocument;
//               it will be deleted after it has been delivered to the
//               process.
////////////////////////////////////////////////////////////////////
void P3DSession::
send_command(TiXmlDocument *command) {
  if (_p3dpython_running) {
    // Python is running.  Send the command.
    nout << "sent: " << *command << "\n" << flush;
    _pipe_write << *command << flush;
    delete command;
  } else {
    // Python not yet running.  Queue up the command instead.
    _commands.push_back(command);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::command_and_response
//       Access: Public
//  Description: Sends the indicated command to the running Python
//               process, and waits for a response.  Returns the
//               newly-allocated response on success, or NULL on
//               failure.
//
//               The command must be a newly-allocated TiXmlDocument;
//               it will be deleted after it has been delivered to the
//               process.
//
//               This will fail if the python process is not running
//               or if it suddenly stops.
////////////////////////////////////////////////////////////////////
TiXmlDocument *P3DSession::
command_and_response(TiXmlDocument *command) {
  if (!_p3dpython_running) {
    return NULL;
  }

  int response_id = _next_response_id;
  ++_next_response_id;

  // Add the "want_response_id" attribute to the toplevel command, so
  // the sub-process knows we'll be waiting for its response.
  TiXmlElement *xcommand = command->FirstChildElement("command");
  assert(xcommand != NULL);
  xcommand->SetAttribute("want_response_id", response_id);

  nout << "sent: " << *command << "\n" << flush;
  _pipe_write << *command << flush;
  delete command;

  // Now block, waiting for a response to be delivered.  We assume
  // only one thread will be waiting at a time.
  _response_ready.acquire();
  while (_response == NULL || _got_response_id != response_id) {
    if (_response != NULL) {
      // This is a bogus response.  Since we're the only thread waiting,
      // it follows that no one is waiting for this response, so we can
      // throw it away.
      nout << "Discarding bogus response: " << *_response << "\n";
      delete _response;
      _response = NULL;
      _got_response_id = -1;
    }

    if (!_p3dpython_running) {
      // Hmm, looks like Python has gone away.

      // TODO: make sure _p3dpython_running gets set to false when the
      // process dies unexpectedly.
      _response_ready.release();
      return NULL;
    }

#ifdef _WIN32
    // Make sure we process the Windows event loop while we're
    // waiting, or everything that depends on Windows messages--in
    // particular, the CreateWindow() call within the subprocess--will
    // starve, and we could end up with deadlock.

    // A single PeekMessage() seems to be sufficient.
    MSG msg;
    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE | PM_NOYIELD);

    // We wait with a timeout, so we can go back and spin the event
    // loop some more.
    _response_ready.wait(0.1);
#else  // _WIN32
    
    // On non-Windows platforms, we can just wait indefinitely.
    _response_ready.wait();
#endif  // _WIN32
  }
  // When we exit the loop, we've found the desired response.

  TiXmlDocument *response = _response;
  _response = NULL;
  _got_response_id = -1;

  _response_ready.release();

  return response;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::xml_to_p3dobj
//       Access: Public
//  Description: Converts the XML representation of the particular
//               object value into a corresponding P3D_object.
//               Returns the newly-allocated object.
////////////////////////////////////////////////////////////////////
P3D_object *P3DSession::
xml_to_p3dobj(const TiXmlElement *xvalue) {
  const char *type = xvalue->Attribute("type");
  if (strcmp(type, "null") == 0) {
    return new P3DNullObject;

  } else if (strcmp(type, "none") == 0) {
    return new P3DNoneObject;

  } else if (strcmp(type, "bool") == 0) {
    int value;
    if (xvalue->QueryIntAttribute("value", &value) == TIXML_SUCCESS) {
      return new P3DBoolObject(value != 0);
    }

  } else if (strcmp(type, "int") == 0) {
    int value;
    if (xvalue->QueryIntAttribute("value", &value) == TIXML_SUCCESS) {
      return new P3DIntObject(value);
    }

  } else if (strcmp(type, "float") == 0) {
    double value;
    if (xvalue->QueryDoubleAttribute("value", &value) == TIXML_SUCCESS) {
      return new P3DFloatObject(value);
    }

  } else if (strcmp(type, "string") == 0) {
    // Using the string form here instead of the char * form, so we
    // don't get tripped up on embedded null characters.
    const string *value = xvalue->Attribute(string("value"));
    if (value != NULL) {
      return new P3DStringObject(*value);
    }

  } else if (strcmp(type, "browser") == 0) {
    int object_id;
    if (xvalue->QueryIntAttribute("object_id", &object_id) == TIXML_SUCCESS) {
      P3D_object *obj = (P3D_object *)object_id;
      return P3D_OBJECT_COPY(obj);
    }

  } else if (strcmp(type, "python") == 0) {
    int object_id;
    if (xvalue->QueryIntAttribute("object_id", &object_id) == TIXML_SUCCESS) {
      return new P3DPythonObject(this, object_id);
    }
  }

  // Something went wrong in decoding.
  return new P3DNoneObject;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::p3dobj_to_xml
//       Access: Public
//  Description: Allocates and returns a new XML structure
//               corresponding to the indicated value.  The supplied
//               P3DObject passed in is *not* deleted.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DSession::
p3dobj_to_xml(const P3D_object *obj) {
  TiXmlElement *xvalue = new TiXmlElement("value");

  switch (P3D_OBJECT_GET_TYPE(obj)) {
  case P3D_OT_null:
    xvalue->SetAttribute("type", "null");
    break;

  case P3D_OT_none:
    xvalue->SetAttribute("type", "none");
    break;

  case P3D_OT_bool:
    xvalue->SetAttribute("type", "bool");
    xvalue->SetAttribute("value", (int)P3D_OBJECT_GET_BOOL(obj));
    break;

  case P3D_OT_int:
    xvalue->SetAttribute("type", "int");
    xvalue->SetAttribute("value", P3D_OBJECT_GET_INT(obj));
    break;

  case P3D_OT_float:
    xvalue->SetAttribute("type", "float");
    xvalue->SetDoubleAttribute("value", P3D_OBJECT_GET_FLOAT(obj));
    break;

  case P3D_OT_string:
    {
      xvalue->SetAttribute("type", "string");
      int size = P3D_OBJECT_GET_STRING(obj, NULL, 0);
      char *buffer = new char[size];
      P3D_OBJECT_GET_STRING(obj, buffer, size);
      xvalue->SetAttribute("value", string(buffer, size));
      delete [] buffer;
    }
    break;

  case P3D_OT_object:
    if (obj->_class == &P3DObject::_object_class) {
      // If it's one of our kind of objects, it must be a
      // P3DPythonObject.  In this case, just send the object_id down,
      // since the actual implementation of this object exists (as a
      // Python object) in the sub-process space.
      int object_id = ((P3DPythonObject *)obj)->get_object_id();
      xvalue->SetAttribute("type", "python");
      xvalue->SetAttribute("object_id", object_id);

    } else {
      // Otherwise, it must a host-provided object, which means we
      // should pass a reference down to this particular object, so
      // the Python process knows to call back up to here to query it.
      // TODO: pass pointers better.  Fix this hideous leak.
      P3D_object *dup = P3D_OBJECT_COPY(obj);
      int object_id = (int)(intptr_t)dup;
      xvalue->SetAttribute("type", "browser");
      xvalue->SetAttribute("object_id", object_id);

      // TODO: manage this reference count somehow.
    }
    break;
  }

  return xvalue;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::install_progress
//       Access: Private
//  Description: Notified as the _panda3d package is downloaded.
////////////////////////////////////////////////////////////////////
void P3DSession::
install_progress(P3DPackage *package, double progress) {
  Instances::iterator ii;
  for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
    P3DInstance *inst = (*ii).second;
    inst->install_progress(package, progress);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::start_p3dpython
//       Access: Private
//  Description: Starts Python running in a child process.
////////////////////////////////////////////////////////////////////
void P3DSession::
start_p3dpython() {
  string p3dpython = P3D_PLUGIN_P3DPYTHON;
  if (p3dpython.empty()) {
    p3dpython = _python_root_dir + "/p3dpython";
#ifdef _WIN32
    p3dpython += ".exe";
#endif
  }

  // Populate the new process' environment.
  string env;

  // These are the enviroment variables we forward from the current
  // environment, if they are set.
  const char *keep[] = {
    "TMP", "TEMP", "HOME", "USER", 
#ifdef _WIN32
    "SYSTEMROOT", "USERPROFILE",
#endif
    NULL
  };
  for (int ki = 0; keep[ki] != NULL; ++ki) {
    char *value = getenv(keep[ki]);
    if (value != NULL) {
      env += keep[ki];
      env += "=";
      env += value;
      env += '\0';
    }
  }

  // Define some new environment variables.
  env += "PATH=";
  env += _python_root_dir;
  env += '\0';

  env += "PYTHONPATH=";
  env += _python_root_dir;
  env += '\0';

  env += "PRC_DIR=";
  env += _python_root_dir;
  env += '\0';

  env += "PANDA_PRC_DIR=";
  env += _python_root_dir;
  env += '\0';

#ifdef _WIN32
  _p3dpython_handle = win_create_process
    (p3dpython, _python_root_dir, env, _output_filename,
     _pipe_read, _pipe_write);
  bool started_p3dpython = (_p3dpython_handle != INVALID_HANDLE_VALUE);
#else
  _p3dpython_pid = posix_create_process
    (p3dpython, _python_root_dir, env, _output_filename,
     _pipe_read, _pipe_write);
  bool started_p3dpython = (_p3dpython_pid > 0);
#endif

  if (!started_p3dpython) {
    nout << "Failed to create process.\n" << flush;
    return;
  }
  _p3dpython_running = true;

  if (!_pipe_read) {
    nout << "unable to open read pipe\n" << flush;
  }
  if (!_pipe_write) {
    nout << "unable to open write pipe\n" << flush;
  }
  
  spawn_read_thread();

  // Now that the process has been started, feed it any commands we
  // may have queued up.
  Commands::iterator ci;
  for (ci = _commands.begin(); ci != _commands.end(); ++ci) {
    nout << "sent: " << *(*ci) << "\n" << flush;
    _pipe_write << *(*ci);
    delete (*ci);
  }
  _pipe_write << flush;
  _commands.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::spawn_read_thread
//       Access: Private
//  Description: Starts the read thread.  This thread is responsible
//               for reading the standard input socket for XML
//               requests and storing them in the _requests queue.
////////////////////////////////////////////////////////////////////
void P3DSession::
spawn_read_thread() {
  assert(!_read_thread_continue);

  _read_thread_continue = true;
  SPAWN_THREAD(_read_thread, rt_thread_run, this);
  _started_read_thread = true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::join_read_thread
//       Access: Private
//  Description: Waits for the read thread to stop.
////////////////////////////////////////////////////////////////////
void P3DSession::
join_read_thread() {
  if (!_started_read_thread) {
    return;
  }

  _read_thread_continue = false;
  _pipe_read.close();

  JOIN_THREAD(_read_thread);
  _started_read_thread = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::rt_thread_run
//       Access: Private
//  Description: The main function for the read thread.
////////////////////////////////////////////////////////////////////
void P3DSession::
rt_thread_run() {
  while (_read_thread_continue) {
    TiXmlDocument *doc = new TiXmlDocument;

    _pipe_read >> *doc;
    if (!_pipe_read || _pipe_read.eof()) {
      // Some error on reading.  Abort.
      rt_terminate();
      return;
    }

    // Successfully read an XML document.
    rt_handle_request(doc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::rt_handle_request
//       Access: Private
//  Description: Processes a single request or notification received
//               from an instance.
////////////////////////////////////////////////////////////////////
void P3DSession::
rt_handle_request(TiXmlDocument *doc) {
  TiXmlElement *xresponse = doc->FirstChildElement("response");
  if (xresponse != (TiXmlElement *)NULL) {
    int response_id;
    if (xresponse->QueryIntAttribute("response_id", &response_id) == TIXML_SUCCESS) {
      // This is a response to a previous command-and-response.  Send
      // it to the parent thread.
      _response_ready.acquire();
      if (_response != NULL) {
        // Hey, there's already a response there.  Since there's only
        // one thread waiting at a time on the command-response cycle,
        // this must be a bogus response that never got picked up.
        // Discard it.
        nout << "Discarding bogus response: " << *_response << "\n";
        delete _response;
      }
      _response = doc;
      _got_response_id = response_id;
      _response_ready.notify();
      _response_ready.release();
      return;
    }
  }

  TiXmlElement *xrequest = doc->FirstChildElement("request");
  if (xrequest != (TiXmlElement *)NULL) {
    int instance_id;
    if (xrequest->QueryIntAttribute("instance_id", &instance_id) == TIXML_SUCCESS) {
      // Look up the particular instance this is related to.
      ACQUIRE_LOCK(_instances_lock);
      Instances::const_iterator ii;
      ii = _instances.find(instance_id);
      if (ii != _instances.end()) {
        P3DInstance *inst = (*ii).second;
        P3D_request *request = rt_make_p3d_request(xrequest);
        if (request != NULL) {
          inst->add_request(request);
        }
      }
      RELEASE_LOCK(_instances_lock);
    }
  }

  delete doc;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::rt_make_p3d_request
//       Access: Private
//  Description: Creates a new P3D_request structure from the XML.
////////////////////////////////////////////////////////////////////
P3D_request *P3DSession::
rt_make_p3d_request(TiXmlElement *xrequest) {
  P3D_request *request = NULL;

  const char *rtype = xrequest->Attribute("rtype");
  if (rtype != NULL) {
    if (strcmp(rtype, "notify") == 0) {
      const char *message = xrequest->Attribute("message");
      if (message != NULL) {
        request = new P3D_request;
        request->_request_type = P3D_RT_notify;
        request->_request._notify._message = strdup(message);
      }

    } else if (strcmp(rtype, "script") == 0) {
      const char *operation = xrequest->Attribute("operation");
      TiXmlElement *xobject = xrequest->FirstChildElement("object");
      const char *property_name = xrequest->Attribute("property_name");
      int needs_response = 0;
      xrequest->Attribute("needs_response", &needs_response);
      int unique_id = 0;
      xrequest->Attribute("unique_id", &unique_id);

      if (operation != NULL && xobject != NULL) {
        P3D_script_operation op;
        if (strcmp(operation, "get_property") == 0) {
          op = P3D_SO_get_property;
        } else if (strcmp(operation, "set_property") == 0) {
          op = P3D_SO_set_property;
        } else if (strcmp(operation, "del_property") == 0) {
          op = P3D_SO_del_property;
        } else if (strcmp(operation, "call") == 0) {
          op = P3D_SO_call;
        } else if (strcmp(operation, "eval") == 0) {
          op = P3D_SO_eval;
        } else {
          // An unexpected operation.
          return NULL;
        }

        request = new P3D_request;
        request->_request_type = P3D_RT_script;
        request->_request._script._op = op;
        request->_request._script._object = xml_to_p3dobj(xobject);
        request->_request._script._property_name = NULL;
        if (property_name != NULL) {
          request->_request._script._property_name = strdup(property_name);
        }
        request->_request._script._needs_response = (needs_response != 0);
        request->_request._script._unique_id = unique_id;
        
        // Fill in the value(s).
        vector<P3D_object *> values;
        TiXmlElement *xvalue = xrequest->FirstChildElement("value");
        while (xvalue != NULL) {
          P3D_object *value = xml_to_p3dobj(xvalue);
          values.push_back(value);
          xvalue = xvalue->NextSiblingElement("value");
        }

        if (values.empty()) {
          // No values.
          request->_request._script._values = NULL;
          request->_request._script._num_values = 0;
        } else {
          // Some values.
          int num_values = (int)values.size();
          P3D_object **valuesp = new P3D_object *[num_values];
          memcpy(valuesp, &values[0], num_values * sizeof(P3D_object *));
          request->_request._script._values = valuesp;
          request->_request._script._num_values = num_values;
        }
      }          

    } else {
      nout << "Ignoring request of type " << rtype << "\n";
    }
  }

  return request;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::rt_terminate
//       Access: Private
//  Description: Got a closed pipe from the sub-process.  Send a
//               terminate request for all instances.
////////////////////////////////////////////////////////////////////
void P3DSession::
rt_terminate() {
  Instances icopy;
  ACQUIRE_LOCK(_instances_lock);
  icopy = _instances;
  RELEASE_LOCK(_instances_lock);

  // TODO: got a race condition here.  What happens if someone deletes
  // an instance while we're processing this loop?

  for (Instances::iterator ii = icopy.begin(); ii != icopy.end(); ++ii) {
    P3DInstance *inst = (*ii).second;
    inst->request_stop();
  }
}

#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: P3DSession::win_create_process
//       Access: Private, Static
//  Description: Creates a sub-process to run the named program
//               executable, with the indicated environment string.
//               Standard error is logged to output_filename, if that
//               string is nonempty.
//
//               Opens the two HandleStreams as the read and write
//               pipes to the child process's standard output and
//               standard input, respectively.
//
//               Returns the handle to the created process on success,
//               or INVALID_HANDLE_VALUE on falure.
////////////////////////////////////////////////////////////////////
HANDLE P3DSession::
win_create_process(const string &program, const string &start_dir,
                   const string &env, const string &output_filename,
                   HandleStream &pipe_read, HandleStream &pipe_write) {

  // Create a bi-directional pipe to communicate with the sub-process.
  HANDLE r_to, w_to, r_from, w_from;

  // Create the pipe to the process.
  if (!CreatePipe(&r_to, &w_to, NULL, 0)) {
    nout << "failed to create pipe\n" << flush;
  } else {
    // Make sure the right end of the pipe is inheritable.
    SetHandleInformation(r_to, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(w_to, HANDLE_FLAG_INHERIT, 0);
  }

  // Create the pipe from the process.
  if (!CreatePipe(&r_from, &w_from, NULL, 0)) {
    nout << "failed to create pipe\n" << flush;
  } else { 
    // Make sure the right end of the pipe is inheritable.
    SetHandleInformation(w_from, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(r_from, HANDLE_FLAG_INHERIT, 0);
  }

  HANDLE error_handle = GetStdHandle(STD_ERROR_HANDLE);
  bool got_output_filename = !output_filename.empty();
  if (got_output_filename) {
    // Open the named file for output and redirect the child's stderr
    // into it.
    HANDLE handle = CreateFile
      (output_filename.c_str(), GENERIC_WRITE, 
       FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
       NULL, CREATE_ALWAYS, 0, NULL);
    if (handle != INVALID_HANDLE_VALUE) {
      error_handle = handle;
      SetHandleInformation(error_handle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    } else {
      nout << "Unable to open " << output_filename << "\n" << flush;
    }
  }

  // Make sure we see an error dialog if there is a missing DLL.
  SetErrorMode(0);

  // Pass the appropriate ends of the bi-directional pipe as the
  // standard input and standard output of the child process.
  STARTUPINFO startup_info;
  ZeroMemory(&startup_info, sizeof(STARTUPINFO));
  startup_info.cb = sizeof(startup_info); 
  startup_info.hStdError = error_handle;
  startup_info.hStdOutput = w_from;
  startup_info.hStdInput = r_to;
  startup_info.dwFlags |= STARTF_USESTDHANDLES;

  // Make sure the "python" console window is hidden.
  startup_info.wShowWindow = SW_HIDE;
  startup_info.dwFlags |= STARTF_USESHOWWINDOW;

  PROCESS_INFORMATION process_info; 
  BOOL result = CreateProcess
    (program.c_str(), NULL, NULL, NULL, TRUE, 0,
     (void *)env.c_str(), start_dir.c_str(),
     &startup_info, &process_info);
  bool started_program = (result != 0);

  // Close the pipe handles that are now owned by the child.
  CloseHandle(w_from);
  CloseHandle(r_to);
  if (got_output_filename) {
    CloseHandle(error_handle);
  }

  if (!started_program) {
    CloseHandle(r_from);
    CloseHandle(w_to);
    return INVALID_HANDLE_VALUE;
  }

  pipe_read.open_read(r_from);
  pipe_write.open_write(w_to);

  CloseHandle(process_info.hThread);
  return process_info.hProcess;
}
#endif // _WIN32


#ifndef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: P3DSession::posix_create_process
//       Access: Private, Static
//  Description: Creates a sub-process to run the named program
//               executable, with the indicated environment string.
//
//               Opens the two HandleStreams as the read and write
//               pipes to the child process's standard output and
//               standard input, respectively.  Returns true on
//               success, false on failure.
//
//               Returns the pid of the created process on success, or
//               -1 on falure.
////////////////////////////////////////////////////////////////////
int P3DSession::
posix_create_process(const string &program, const string &start_dir,
                     const string &env, const string &output_filename,
                     HandleStream &pipe_read, HandleStream &pipe_write) {
  // Create a bi-directional pipe to communicate with the sub-process.
  int to_fd[2];
  if (pipe(to_fd) < 0) {
    perror("failed to create pipe");
  }
  int from_fd[2];
  if (pipe(from_fd) < 0) {
    perror("failed to create pipe");
  }

  // Fork and exec.
  pid_t child = fork();
  if (child < 0) {
    close(to_fd[0]);
    close(to_fd[1]);
    close(from_fd[0]);
    close(from_fd[1]);
    perror("fork");
    return -1;
  }

  if (child == 0) {
    // Here we are in the child process.
    bool got_output_filename = !output_filename.empty();
    if (got_output_filename) {
      // Open the named file for output and redirect the child's stderr
      // into it.
      int logfile_fd = open(output_filename.c_str(), 
                            O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (logfile_fd < 0) {
        nout << "Unable to open " << output_filename << "\n" << flush;
      } else {
        dup2(logfile_fd, STDERR_FILENO);
        close(logfile_fd);
      }
    }

    // Set the appropriate ends of the bi-directional pipe as the
    // standard input and standard output of the child process.
    dup2(to_fd[0], STDIN_FILENO);
    dup2(from_fd[1], STDOUT_FILENO);
    close(to_fd[1]);
    close(from_fd[0]);

    if (chdir(start_dir.c_str()) < 0) {
      nout << "Could not chdir to " << start_dir << "\n" << flush;
      _exit(1);
    }

    // build up an array of char strings for the environment.
    vector<const char *> ptrs;
    size_t p = 0;
    size_t zero = env.find('\0', p);
    while (zero != string::npos) {
      ptrs.push_back(env.data() + p);
      p = zero + 1;
      zero = env.find('\0', p);
    }
    ptrs.push_back((char *)NULL);
    
    execle(program.c_str(), program.c_str(), (char *)0, &ptrs[0]);
    nout << "Failed to exec " << program << "\n" << flush;
    _exit(1);
  }

  pipe_read.open_read(from_fd[0]);
  pipe_write.open_write(to_fd[1]);
  close(to_fd[0]);
  close(from_fd[1]);

  return child;
}
#endif  // _WIN32


////////////////////////////////////////////////////////////////////
//     Function: P3DSession::PackageCallback::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DSession::PackageCallback::
PackageCallback(P3DSession *session) :
  _session(session)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::PackageCallback::package_ready
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DSession::PackageCallback::
package_ready(P3DPackage *package, bool success) {
  if (this == _session->_panda3d_callback) {
    _session->_panda3d_callback = NULL;
    if (package == _session->_panda3d) {
      if (success) {
        _session->start_p3dpython();
      } else {
        nout << "Failed to install " << package->get_package_name()
             << "_" << package->get_package_version() << "\n";
      }
    } else {
      nout << "Unexpected panda3d package: " << package << "\n";
    }
  } else {
    nout << "Unexpected callback for P3DSession\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::PackageCallback::install_progress
//       Access: Public, Virtual
//  Description: This callback is received during the download process
//               to inform us how much has been installed so far.
////////////////////////////////////////////////////////////////////
void P3DSession::PackageCallback::
install_progress(P3DPackage *package, double progress) {
  if (this == _session->_panda3d_callback) {
    _session->install_progress(package, progress);
  }
}
