// Filename: deadrec_send.cxx
// Created by:  cary (15Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "framework.h"
#include <renderRelation.h>
#include <queuedConnectionManager.h>
#include <modelPool.h>
#include <transformTransition.h>
#include <lerp.h>
#include <guiFrame.h>
#include <guiButton.h>
#include <guiSign.h>

#include <dconfig.h>

NotifyCategoryDeclNoExport(deadrec);
NotifyCategoryDef(deadrec, "");

Configure(deadrec);

ConfigureFn(deadrec) {
}

static PT_Node smiley;
static RenderRelation* my_arc;
static LPoint3f my_pos;
static LVector3f my_vel;
string hostname = deadrec.GetString("deadrec-rec", "localhost");
int hostport = deadrec.GetInt("deadrec-rec-port", 0xdead);

static QueuedConnectionManager cm;
PT(Connection) conn;
ConnectionWriter* writer;

enum TelemetryToken { T_End = 1, T_Pos, T_Vel, T_Num };

static inline NetDatagram& add_pos(NetDatagram& d) {
  d.add_uint8(T_Pos);
  d.add_float64(my_pos[0]);
  d.add_float64(my_pos[1]);
  d.add_float64(my_pos[2]);
  return d;
}

static inline NetDatagram& add_vel(NetDatagram& d) {
  d.add_uint8(T_Vel);
  d.add_float64(my_vel[0]);
  d.add_float64(my_vel[1]);
  d.add_float64(my_vel[2]);
  return d;
}

static inline void send(NetDatagram& d) {
  d.add_uint8(T_End);
  writer->send(d, conn);
}

static void event_frame(CPT_Event) {
  // send deadrec data
  NetDatagram d;
  send(add_pos(d));
}

enum MotionType { M_None, M_Line, M_SLine, M_Box, M_Circle, M_Random };
PT(AutonomousLerp) curr_lerp;
MotionType curr_type;
PT(GuiButton) lineButton;
PT(GuiButton) slineButton;
PT(GuiButton) boxButton;
PT(GuiButton) circleButton;
PT(GuiButton) randomButton;

// the various motion generators

void update_smiley(void) {
  LMatrix4f mat = LMatrix4f::translate_mat(my_pos);
  my_arc->set_transition(new TransformTransition(mat));
}

class MyPosFunctor : public LPoint3fLerpFunctor {
public:
  MyPosFunctor(LPoint3f start, LPoint3f end) : LPoint3fLerpFunctor(start,
								   end) {}
  MyPosFunctor(const MyPosFunctor& p) : LPoint3fLerpFunctor(p) {}
  virtual ~MyPosFunctor(void) {}
  virtual void operator()(float t) {
    LPoint3f p = interpolate(t);
    my_vel = p - my_pos;
    my_pos = p;
    update_smiley();
  }
public:
  // type stuff
  static TypeHandle get_class_type(void) { return _type_handle; }
  static void init_type(void) {
    LPoint3fLerpFunctor::init_type();
    register_type(_type_handle, "MyPosFunctor",
		  LPoint3fLerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type(void) const { return get_class_type(); }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};
TypeHandle MyPosFunctor::_type_handle;

static void init_funcs(void) {
  static bool inited = false;
  if (!inited) {
    MyPosFunctor::init_type();
    inited = true;
  }
}

static void run_line(bool smooth) {
  static bool where = false;
  LerpBlendType* blend;

  init_funcs();
  if (smooth)
    blend = new EaseInOutBlendType();
  else
    blend = new NoBlendType();
  if (where) {
    curr_lerp =
      new AutonomousLerp(new MyPosFunctor(my_pos, LPoint3f::rfu(10., 0., 0.)),
			 5., blend, &event_handler);
    curr_lerp->set_end_event("lerp_done");
    curr_lerp->start();
  } else {
    curr_lerp =
      new AutonomousLerp(new MyPosFunctor(my_pos, LPoint3f::rfu(-10., 0., 0.)),
			 5., blend, &event_handler);
    curr_lerp->set_end_event("lerp_done");
    curr_lerp->start();
  }
  where = !where;
}

static void run_box(bool smooth) {
  static int where = 0;
  LerpBlendType* blend;

  init_funcs();
  if (smooth)
    blend = new EaseInOutBlendType();
  else
    blend = new NoBlendType();
  switch (where) {
  case 0:
    curr_lerp =
      new AutonomousLerp(new MyPosFunctor(my_pos,
					  LPoint3f::rfu(-10., 0., 10.)),
			 5., blend, &event_handler);
    where = 1;
    break;
  case 1:
    curr_lerp =
      new AutonomousLerp(new MyPosFunctor(my_pos, LPoint3f::rfu(10., 0., 10.)),
			 5., blend, &event_handler);
    where = 2;
    break;
  case 2:
    curr_lerp =
      new AutonomousLerp(new MyPosFunctor(my_pos,
					  LPoint3f::rfu(10., 0., -10.)),
			 5., blend, &event_handler);
    where = 3;
    break;
  case 3:
    curr_lerp =
      new AutonomousLerp(new MyPosFunctor(my_pos,
					  LPoint3f::rfu(-10., 0., -10.)),
			 5., blend, &event_handler);
    where = 0;
    break;
  default:
    deadrec_cat->error() << "I'm a tard and box::where got out of range ("
			 << where << ")" << endl;
    where = 0;
    run_box(smooth);
  }
  curr_lerp->set_end_event("lerp_done");
  curr_lerp->start();
}

static void run_circle(bool smooth) {
  init_funcs();
}

static void run_random(bool smooth) {
  init_funcs();
}

static void handle_lerp(void) {
  if (curr_lerp != (AutonomousLerp*)0L)
    curr_lerp->stop();
  curr_lerp = (AutonomousLerp*)0L;
  switch (curr_type) {
  case M_None:
    break;
  case M_Line:
    run_line(false);
    break;
  case M_SLine:
    run_line(true);
    break;
  case M_Box:
    run_box(false);
    break;
  case M_Circle:
    run_circle(false);
    break;
  case M_Random:
    run_random(false);
    break;
  default:
    deadrec_cat->error() << "unknown motion type (" << (int)curr_type << ")"
			 << endl;
  }
}

static void make_active(void) {
  switch (curr_type) {
  case M_None:
    break;
  case M_Line:
    lineButton->up();
    break;
  case M_SLine:
    slineButton->up();
    break;
  case M_Box:
    boxButton->up();
    break;
  case M_Circle:
    circleButton->up();
    break;
  case M_Random:
    randomButton->up();
    break;
  default:
    deadrec_cat->error() <<" unknown motion type (" << (int)curr_type << ")"
			 << endl;
  }
}

static void event_button_down(CPT_Event e) {
  string s = e->get_name();
  s = s.substr(0, s.find("-down"));
  if (s == "line") {
    if (curr_type != M_Line) {
      make_active();
      curr_type = M_Line;
      handle_lerp();
      lineButton->inactive();
    }
  } else if (s == "s-line") {
    if (curr_type != M_SLine) {
      make_active();
      curr_type = M_SLine;
      handle_lerp();
      slineButton->inactive();
    }
  } else if (s == "box") {
    if (curr_type != M_Box) {
      make_active();
      curr_type = M_Box;
      handle_lerp();
      boxButton->inactive();
    }
  } else if (s == "circle") {
    if (curr_type != M_Circle) {
      make_active();
      curr_type = M_Circle;
      handle_lerp();
      circleButton->inactive();
    }
  } else if (s == "random") {
    if (curr_type != M_Random) {
      make_active();
      curr_type = M_Random;
      handle_lerp();
      randomButton->inactive();
    }
  } else {
    deadrec_cat->error() << "got invalid button event '" << s << "'" << endl;
  }
}

static inline GuiButton* make_button(const string& name, Node* font,
				     EventHandler& eh) {
  GuiLabel* l1 = GuiLabel::make_simple_text_label(name, font);
  GuiLabel* l2 = GuiLabel::make_simple_text_label(name, font);
  GuiLabel* l3 = GuiLabel::make_simple_text_label(name, font);
  GuiLabel* l4 = GuiLabel::make_simple_text_label(name, font);
  GuiLabel* l5 = GuiLabel::make_simple_text_label(name, font);
  // up
  l1->set_background_color(1., 1., 1., 0.);
  // up-rollover
  l2->set_background_color(1., 1., 1., 0.5);
  // down
  l3->set_background_color(1., 1., 1., 0.);
  // down-rollover
  l4->set_background_color(1., 1., 1., 0.5);
  // 'inactive'
  l5->set_background_color(1., 0., 0., 0.7);
  GuiButton* b1 = new GuiButton(name, l1, l2, l3, l4, l5);
  eh.add_hook(name + "-down", event_button_down);
  eh.add_hook(name + "-down-rollover", event_button_down);
  return b1;
}

static void deadrec_setup(EventHandler& eh) {
  static bool done = false;
  if (done)
    return;
  // load smiley and put it in the scenegraph
  smiley = ModelPool::load_model("smiley");
  nassertv(smiley != (Node*)0L);
  my_arc = new RenderRelation(render, smiley);
  // open a connection to the receiver
  NetAddress host;
  if (!host.set_host(hostname, hostport)) {
    deadrec_cat->fatal() << "Unknown host: " << hostname << endl;
    exit(0);
  }
  conn = cm.open_TCP_client_connection(host, 5000);
  if (conn.is_null()) {
    deadrec_cat->fatal() << "no connection." << endl;
    exit(0);
  }
  if (deadrec_cat->is_debug())
    deadrec_cat->debug() << "opened TCP connection to " << hostname
			 << " on port " << conn->get_address().get_port()
			 << " and IP " << conn->get_address() << endl;
  writer = new ConnectionWriter(&cm, 0);

  // create an interface
  GuiManager* mgr = GuiManager::get_ptr(main_win, mak);
  PT_Node font = ModelPool::load_model("ttf-comic");
  GuiFrame* f1 = new GuiFrame("motions");
  GuiLabel* l1 = GuiLabel::make_simple_text_label("motion:", font);
  l1->set_background_color(1., 1., 1., 0.);
  GuiSign* s1 = new GuiSign("motion", l1);
  s1->set_scale(0.08);
  f1->add_item(s1);
  lineButton = make_button("line", font, eh);
  lineButton->set_scale(0.08);
  f1->add_item(lineButton);
  slineButton = make_button("s-line", font, eh);
  slineButton->set_scale(0.08);
  f1->add_item(slineButton);
  boxButton = make_button("box", font, eh);
  boxButton->set_scale(0.08);
  f1->add_item(boxButton);
  circleButton = make_button("circle", font, eh);
  circleButton->set_scale(0.08);
  f1->add_item(circleButton);
  randomButton = make_button("random", font, eh);
  randomButton->set_scale(0.08);
  f1->add_item(randomButton);
  f1->pack_item(lineButton, GuiFrame::UNDER, s1);
  f1->pack_item(lineButton, GuiFrame::ALIGN_LEFT, s1);
  f1->pack_item(slineButton, GuiFrame::UNDER, s1);
  f1->pack_item(slineButton, GuiFrame::RIGHT, lineButton, 0.02);
  f1->pack_item(boxButton, GuiFrame::UNDER, s1);
  f1->pack_item(boxButton, GuiFrame::RIGHT, slineButton, 0.02);
  f1->pack_item(circleButton, GuiFrame::UNDER, s1);
  f1->pack_item(circleButton, GuiFrame::RIGHT, boxButton, 0.02);
  f1->pack_item(randomButton, GuiFrame::UNDER, s1);
  f1->pack_item(randomButton, GuiFrame::RIGHT, circleButton, 0.02);
  f1->set_pos(LVector3f::rfu(-0.1, 0., 0.9));
  f1->recompute();
  f1->manage(mgr, eh);
}

static void event_lerp(CPT_Event) {
  handle_lerp();
}

static void deadrec_keys(EventHandler& eh) {
  deadrec_setup(eh);

  eh.add_hook("NewFrame", event_frame);
  eh.add_hook("lerp_done", event_lerp);
}

int main(int argc, char* argv[]) {
  define_keys = &deadrec_keys;
  return framework_main(argc, argv);
}
