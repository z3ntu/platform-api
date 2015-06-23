Ubuntu Platform API {#mainpage}
===================

The Ubuntu platform API Implements access to the Ubuntu platform and
is the primary carrier across form-factor boundaries. It serves as a
low-level access layer to the underlying system and its capabilities.

Intended Audience
-----------------

The intended audience of this API and its documentation are
integrators and developers who either cannot or do not want to rely on
the more convenient QML or HTML5/JS SDKs.

Source Tree Layout
------------------

The overall source tree is split up into roughly two parts:

  * include/
    * ubuntu/
      * application/
        * sensors/
  * android/

where include contains all of the public types, functions and
interfaces offered by the Ubuntu platform API. The android subfolder
contains the android-specific implementation of the Ubuntu platform
API, together with a default implementation of the C API on top of the
generic C++ API. For developers, only the include/ folder and its
subdirectories are of interest.

Within the include/ folder, only the ubuntu/application/ subfolder is
meant for public consumption.
