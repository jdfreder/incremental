// Filename: fltRecordReader.h
// Created by:  drose (24Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef FLTRECORDREADER_H
#define FLTRECORDREADER_H

#include <pandatoolbase.h>

#include "fltOpcode.h"
#include "fltError.h"

#include <datagram.h>
#include <datagramIterator.h>

////////////////////////////////////////////////////////////////////
//       Class : FltRecordReader
// Description : This class turns an istream into a sequence of
//               FltRecords by reading a sequence of Datagrams and
//               extracting the opcode from each one.  It remembers
//               where it is in the file and what the current record
//               is.
////////////////////////////////////////////////////////////////////
class FltRecordReader {
public:
  FltRecordReader(istream &in);
  ~FltRecordReader();

  FltOpcode get_opcode() const;
  DatagramIterator &get_iterator();
  const Datagram &get_datagram();
  int get_record_length() const;

  FltError advance(bool ok_eof = false);

  bool eof() const;
  bool error() const;

private:
  void read_next_header();

  istream &_in;
  Datagram _datagram;
  FltOpcode _opcode;
  int _record_length;
  DatagramIterator *_iterator;
  
  FltError _next_error;
  FltOpcode _next_opcode;
  int _next_record_length;

  enum State {
    S_begin,
    S_normal,
    S_eof,
    S_error
  };
  State _state;
};

#endif


