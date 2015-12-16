Scriptable avionics modeling suite for flight simulators.

SASL features scriptable component model and flight simulation abstraction layer.  Simulated avionics can be connected to virtually any flight simulator.  Current version has networked X-Plane connector and X-Plane plugin which integrates SASL avionics and X-Plane aircrafts.

Development snapshot builds consists of following components:

  1. xap: X-Plane plugin for aircrafts authors.  If you are not author it is not necessary to download it.
  1. slava: Stand-Alone Avionics Application.  slava connects to flight simulator and shows stand-alone panels.  It is lacks GUI at this stage of development so users have to edit launch scripts to specify parameters like simulator host address and password.  Example launch scripts for Windows and Linux as well as sample navigator panel provided within slava package.
  1. Tu-104 example aircraft.  This X-Plane aircraft shows some features of sasl.  If you just interested in flying Tu-104 it is only file you have to download.  To activate popup navigator panel just click "N" button on aircraft panel.

If you are interested in sasl development or have some questions about this project don't hesitate to contact project admin.