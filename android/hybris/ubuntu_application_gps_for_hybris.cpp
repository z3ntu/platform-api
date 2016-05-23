/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Daniel d'Andrada <daniel.dandrada@canonical.com>
 */
#include <ubuntu/hardware/gps.h>

#include <pthread.h>

// android stuff
#include <hardware/gps.h>
#include <hardware_legacy/power.h>

#include <unistd.h>

#define WAKE_LOCK_NAME  "U_HARDWARE_GPS"

struct UHardwareGps_
{
    UHardwareGps_(UHardwareGpsParams* params);
    ~UHardwareGps_();

    bool init();

    bool start();
    bool stop();
    void inject_time(int64_t time, int64_t timeReference, int uncertainty);
    void inject_location(double latitude, double longitude, float accuracy);
    void delete_aiding_data(UHardwareGpsAidingData flags);

    void set_server_for_type(UHardwareGpsAGpsType type, const char* hostname, uint16_t port);
    void set_reference_location(UHardwareGpsAGpsRefLocation* location, size_t size_of_struct);

    void notify_connection_is_open(const char* apn);
    void notify_connection_is_closed();
    void notify_connection_not_available();

    bool set_position_mode(uint32_t mode, uint32_t recurrence, uint32_t min_interval,
                           uint32_t preferred_accuracy, uint32_t preferred_time);
    void inject_xtra_data(char* data, int length);

    const GpsInterface* gps_interface;
    const GpsXtraInterface* gps_xtra_interface;
    const AGpsInterface* agps_interface;
    const GpsNiInterface* gps_ni_interface;
    const GpsDebugInterface* gps_debug_interface;
    const AGpsRilInterface* agps_ril_interface;

    UHardwareGpsLocationCallback location_cb;
    UHardwareGpsStatusCallback status_cb;
    UHardwareGpsSvStatusCallback sv_status_cb;
    UHardwareGpsNmeaCallback nmea_cb;
    UHardwareGpsSetCapabilities set_capabilities_cb;
    UHardwareGpsRequestUtcTime request_utc_time_cb;

    UHardwareGpsXtraDownloadRequest xtra_download_request_cb;

    UHardwareGpsAGpsStatusCallback agps_status_cb;

    UHardwareGpsNiNotifyCallback gps_ni_notify_cb;

    UHardwareGpsAGpsRilRequestSetId request_setid_cb;
    UHardwareGpsAGpsRilRequestRefLoc request_refloc_cb;

    void* context;
};

namespace
{
UHardwareGps hybris_gps_instance = NULL;

namespace cb
{
static void location(GpsLocation* location)
{
    if (hybris_gps_instance && hybris_gps_instance->location_cb)
        hybris_gps_instance->location_cb(
            reinterpret_cast<UHardwareGpsLocation*>(location),
            hybris_gps_instance->context);
}

static void status(GpsStatus* status)
{
    if (hybris_gps_instance && hybris_gps_instance->status_cb)
        hybris_gps_instance->status_cb(status->status, hybris_gps_instance->context);
}

static void sv_status(GpsSvStatus* sv_status)
{
    if (hybris_gps_instance && hybris_gps_instance->sv_status_cb)
        hybris_gps_instance->sv_status_cb(
            reinterpret_cast<UHardwareGpsSvStatus*>(sv_status),
            hybris_gps_instance->context);
}

#ifdef BOARD_HAS_GNSS_STATUS_CALLBACK
static void gnss_sv_status(GnssSvStatus*)
{
    // Empty on purpose. We do not expose status information about
    // other satellites networks, yet.
}
#endif

static void nmea(GpsUtcTime timestamp, const char* nmea, int length)
{
    if (hybris_gps_instance && hybris_gps_instance->nmea_cb)
        hybris_gps_instance->nmea_cb(timestamp, nmea, length, hybris_gps_instance->context);
}

static void set_capabilities(uint32_t capabilities)
{
    if (hybris_gps_instance && hybris_gps_instance->set_capabilities_cb)
        hybris_gps_instance->set_capabilities_cb(capabilities, hybris_gps_instance->context);
}

static void acquire_wakelock()
{
    acquire_wake_lock(PARTIAL_WAKE_LOCK, WAKE_LOCK_NAME);
}

static void release_wakelock()
{
    release_wake_lock(WAKE_LOCK_NAME);
}

static void request_utc_time()
{
    if (hybris_gps_instance && hybris_gps_instance->request_utc_time_cb)
        hybris_gps_instance->request_utc_time_cb(hybris_gps_instance->context);
}

typedef struct 
{
    void (*func)(void *);
    void *arg;
} FuncAndArg;

static void* thread_start_wrapper(void* arg)
{
    FuncAndArg *func_and_arg = reinterpret_cast<FuncAndArg*>(arg);
    func_and_arg->func(func_and_arg->arg);
    delete func_and_arg;
    return NULL;
}

static pthread_t create_thread(const char* name, void (*start)(void *), void* arg)
{
    pthread_t thread;

    FuncAndArg *func_and_arg = new FuncAndArg;
    func_and_arg->func = start;
    func_and_arg->arg = arg;

    pthread_create(&thread, NULL, thread_start_wrapper, func_and_arg);
    return thread;
}

GpsCallbacks gps =
{
    sizeof(GpsCallbacks),
    cb::location,
    cb::status,
    cb::sv_status,
#ifdef BOARD_HAS_GNSS_STATUS_CALLBACK
    cb::gnss_sv_status,
#endif
    cb::nmea,
    cb::set_capabilities,
    cb::acquire_wakelock,
    cb::release_wakelock,
    cb::create_thread,
    cb::request_utc_time,
};

static void xtra_download_request()
{
    if (hybris_gps_instance && hybris_gps_instance->xtra_download_request_cb)
        hybris_gps_instance->xtra_download_request_cb(hybris_gps_instance->context);
}

GpsXtraCallbacks gps_xtra =
{
    cb::xtra_download_request,
    cb::create_thread,
};

static void agps_status(AGpsStatus* agps_status)
{
    if (hybris_gps_instance && hybris_gps_instance->agps_status_cb)
        hybris_gps_instance->agps_status_cb(
            reinterpret_cast<UHardwareGpsAGpsStatus*>(agps_status), hybris_gps_instance->context);
}

AGpsCallbacks agps =
{
    cb::agps_status,
    cb::create_thread
};

static void gps_ni_notify(GpsNiNotification *notification)
{
    if (hybris_gps_instance && hybris_gps_instance->gps_ni_notify_cb)
        hybris_gps_instance->gps_ni_notify_cb(
            reinterpret_cast<UHardwareGpsNiNotification*>(notification),
            hybris_gps_instance->context);
}

GpsNiCallbacks gps_ni =
{
    cb::gps_ni_notify,
    cb::create_thread,
};

static void agps_request_set_id(uint32_t flags)
{
    if (hybris_gps_instance && hybris_gps_instance->request_setid_cb)
        hybris_gps_instance->request_setid_cb(flags, hybris_gps_instance->context);
}

static void agps_request_ref_location(uint32_t flags)
{
    if (hybris_gps_instance && hybris_gps_instance->request_refloc_cb)
        hybris_gps_instance->request_refloc_cb(flags, hybris_gps_instance->context);
}

AGpsRilCallbacks agps_ril =
{
    cb::agps_request_set_id,
    cb::agps_request_ref_location,
    cb::create_thread,
};
}
}

UHardwareGps_::UHardwareGps_(UHardwareGpsParams* params)
    : gps_interface(NULL),
      gps_xtra_interface(NULL),
      agps_interface(NULL),
      gps_ni_interface(NULL),
      gps_debug_interface(NULL),
      agps_ril_interface(NULL),
      location_cb(params->location_cb),
      status_cb(params->status_cb),
      sv_status_cb(params->sv_status_cb),
      nmea_cb(params->nmea_cb),
      set_capabilities_cb(params->set_capabilities_cb),
      request_utc_time_cb(params->request_utc_time_cb),
      xtra_download_request_cb(params->xtra_download_request_cb),
      agps_status_cb(params->agps_status_cb),
      gps_ni_notify_cb(params->gps_ni_notify_cb),
      request_setid_cb(params->request_setid_cb),
      request_refloc_cb(params->request_refloc_cb),
      context(params->context)
{
   
}

UHardwareGps_::~UHardwareGps_()
{
    if (gps_interface)
        gps_interface->cleanup();
}

bool UHardwareGps_::init()
{
    int err;
    hw_module_t* module;

    err = hw_get_module(GPS_HARDWARE_MODULE_ID, (hw_module_t const**)&module);

    if (err != 0) return false;

    hw_device_t* device;
    err = module->methods->open(module, GPS_HARDWARE_MODULE_ID, &device);

    if (err != 0) return false;

    gps_device_t* gps_device = (gps_device_t *)device;
    gps_interface = gps_device->get_gps_interface(gps_device);

    if (not gps_interface) return false;
    if (gps_interface->init(&cb::gps) != 0) return false;
    
    gps_xtra_interface =
            (const GpsXtraInterface*)gps_interface->get_extension(GPS_XTRA_INTERFACE);
    agps_interface =
            (const AGpsInterface*)gps_interface->get_extension(AGPS_INTERFACE);
    gps_ni_interface =
            (const GpsNiInterface*)gps_interface->get_extension(GPS_NI_INTERFACE);
    gps_debug_interface =
            (const GpsDebugInterface*)gps_interface->get_extension(GPS_DEBUG_INTERFACE);
    agps_ril_interface =
            (const AGpsRilInterface*)gps_interface->get_extension(AGPS_RIL_INTERFACE);
    
    // if XTRA initialization fails we will disable it by gps_Xtra_interface to null,
    // but continue to allow the rest of the GPS interface to work.
    if (gps_xtra_interface && gps_xtra_interface->init(&cb::gps_xtra) != 0)
        gps_xtra_interface = NULL;
    if (agps_interface)
        agps_interface->init(&cb::agps);
    if (gps_ni_interface)
        gps_ni_interface->init(&cb::gps_ni);
    if (agps_ril_interface)
        agps_ril_interface->init(&cb::agps_ril);

    return true;
}

bool UHardwareGps_::start()
{
    if (gps_interface)
        return (gps_interface->start() == 0);
    else
        return false;
}

bool UHardwareGps_::stop()
{
    if (gps_interface)
        return (gps_interface->stop() == 0);
    else
        return false;
}

void UHardwareGps_::inject_time(int64_t time, int64_t time_reference, int uncertainty)
{
    if (gps_interface)
        gps_interface->inject_time(time, time_reference, uncertainty);
}

void UHardwareGps_::inject_location(double latitude, double longitude, float accuracy)
{
    if (gps_interface && gps_interface->inject_location)
        gps_interface->inject_location(latitude, longitude, accuracy);
}

void UHardwareGps_::delete_aiding_data(uint16_t flags)
{
    if (gps_interface)
        gps_interface->delete_aiding_data(flags);
}

void UHardwareGps_::set_server_for_type(UHardwareGpsAGpsType type, const char* hostname, uint16_t port)
{
    if (agps_interface && agps_interface->set_server)
        agps_interface->set_server(type, hostname, port);
}

void UHardwareGps_::set_reference_location(UHardwareGpsAGpsRefLocation* location, size_t size_of_struct)
{
    AGpsRefLocation ref_loc;
    ref_loc.type = location->type;
    ref_loc.u.cellID.type = location->u.cellID.type;
    ref_loc.u.cellID.mcc = location->u.cellID.mcc;
    ref_loc.u.cellID.mnc = location->u.cellID.mnc;
    ref_loc.u.cellID.lac = location->u.cellID.lac;
    ref_loc.u.cellID.cid = location->u.cellID.cid;

    if (agps_ril_interface && agps_ril_interface->set_ref_location)
        agps_ril_interface->set_ref_location(&ref_loc, sizeof(ref_loc));
}

void UHardwareGps_::notify_connection_is_open(const char* apn)
{
    if (agps_interface && agps_interface->data_conn_open)
        agps_interface->data_conn_open(apn);
}

void UHardwareGps_::notify_connection_is_closed()
{
    if (agps_interface && agps_interface->data_conn_closed)
        agps_interface->data_conn_closed();
}

void UHardwareGps_::notify_connection_not_available()
{
    if (agps_interface && agps_interface->data_conn_failed)
        agps_interface->data_conn_failed();
}

bool UHardwareGps_::set_position_mode(uint32_t mode, uint32_t recurrence, uint32_t min_interval,
                                    uint32_t preferred_accuracy, uint32_t preferred_time)
{
    if (gps_interface)
        return (gps_interface->set_position_mode(mode, recurrence, min_interval,
                                                 preferred_accuracy, preferred_time) == 0);
    else
        return false;
}

void UHardwareGps_::inject_xtra_data(char* data, int length)
{
    if (gps_xtra_interface)
        gps_xtra_interface->inject_xtra_data(data, length);
}

/////////////////////////////////////////////////////////////////////
// Implementation of the C API

UHardwareGps u_hardware_gps_new(UHardwareGpsParams* params)
{
    if (hybris_gps_instance != NULL)
        return NULL;

    UHardwareGps u_hardware_gps = new UHardwareGps_(params);
    hybris_gps_instance = u_hardware_gps;

    // Try ten times to initialize the GPS HAL interface,
    // sleeping for 200ms per iteration in case of issues.
    for (unsigned int i = 0; i < 50; i++)
        if (u_hardware_gps->init())
            return hybris_gps_instance = u_hardware_gps;
        else
            // Sleep for some time and leave some time for the system
            // to finish initialization.
            ::usleep(200 * 1000);

    // This is the error case, as we did not succeed in initializing the GPS interface.
    delete u_hardware_gps;
    return hybris_gps_instance;
}

void u_hardware_gps_delete(UHardwareGps handle)
{
    delete handle;
    if (handle == hybris_gps_instance)
        hybris_gps_instance = NULL;
}

bool u_hardware_gps_start(UHardwareGps self)
{
    return self->start();
}

bool u_hardware_gps_stop(UHardwareGps self)
{
    return self->stop();
}

void u_hardware_gps_inject_time(UHardwareGps self, int64_t time, int64_t time_reference,
                            int uncertainty)
{
    self->inject_time(time, time_reference, uncertainty);
}

void u_hardware_gps_inject_location(UHardwareGps self,
                                    UHardwareGpsLocation location)
{
    self->inject_location(location.latitude, location.longitude,
                          location.accuracy);
}

void u_hardware_gps_delete_aiding_data(UHardwareGps self, UHardwareGpsAidingData flags)
{
    self->delete_aiding_data(flags);
}

void u_hardware_gps_agps_set_server_for_type(
        UHardwareGps self,
        UHardwareGpsAGpsType type,
        const char* hostname,
        uint16_t port)
{
    self->set_server_for_type(type, hostname, port);
}

void u_hardware_gps_agps_set_reference_location(
    UHardwareGps self,
    UHardwareGpsAGpsRefLocation *location,
    size_t size_of_struct)
{
    self->set_reference_location(location, size_of_struct);
}

void u_hardware_gps_agps_notify_connection_is_open(
    UHardwareGps self,
    const char *apn)
{
    self->notify_connection_is_open(apn);
}

void u_hardware_gps_agps_notify_connection_is_closed(UHardwareGps self)
{
    self->notify_connection_is_closed();
}

void u_hardware_gps_agps_notify_connection_not_available(UHardwareGps self)
{
    self->notify_connection_not_available();
}


bool u_hardware_gps_set_position_mode(UHardwareGps self, uint32_t mode, uint32_t recurrence,
                                  uint32_t min_interval, uint32_t preferred_accuracy,
                                  uint32_t preferred_time)
{
    return self->set_position_mode(mode, recurrence, min_interval, preferred_accuracy,
                                   preferred_time);
}

void u_hardware_gps_inject_xtra_data(UHardwareGps self, char* data, int length)
{
    self->inject_xtra_data(data, length);
}
