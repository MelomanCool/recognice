#include <string>
#include <memory>
#include <functional>
#include <stdexcept>

#include <gnsdk.hpp>
#include <pybind11/pybind11.h>

#include "StreamEvents.h"
#include "UserStore.h"

using namespace gracenote;
using gracenote::musicid_stream::GnMusicIdStream;
using gracenote::musicid_stream::GnMusicIdStreamPreset;
using gracenote::metadata::GnTrack;
using gracenote::metadata::GnRenderOptions;
using gracenote::GnLookupData;
using metadata::GnResponseAlbums;

namespace py = pybind11;
using namespace pybind11::literals;


gnsdk_cstr_t APPLICATION_VERSION = "0.1";


void enable_logging(std::string filename) {
    GnLog log(
         filename.c_str(),
         GnLogFilters().Error().Warning(),
         GnLogColumns().All(),
         GnLogOptions().MaxSize(0).Archive(false),
         GNSDK_NULL);
    log.Enable(kLogPackageAllGNSDK);
}

std::unique_ptr<GnManager> init_gracenote(std::string license_text)
{
    std::unique_ptr<GnManager> mgr(new GnManager(license_text.c_str(), kLicenseInputModeString));
    return mgr;
}

void load_locale(const GnUser& user)
{
    GnLocale(
       GnLocaleGroup::kLocaleGroupMusic,
       GnLanguage::kLanguageRussian,
       GnRegion::kRegionEurope,
       GnDescriptor::kDescriptorDefault,
       user)
    .SetGroupDefault();
}

GnUser create_or_load_user(const std::string& client_id,
                           const std::string& client_tag,
                           const int user_id)
{
    UserStore user_store(user_id);
    GnUser user = GnUser(client_id.c_str(), client_tag.c_str(), APPLICATION_VERSION, &user_store);
    load_locale(user);
    return user;
}


py::dict recognize(const GnUser& user,
                  const std::string& audio_chunk,
                  const py::object& logger)
{
    StreamEvents<std::string> stream_events([](GnResponseAlbums& albums) {
        return std::string(albums.Render(GnRenderOptions()
                                         .Json()
                                         .Full()));
    });

    GnMusicIdStream ident(user, kPresetRadio, &stream_events);

//    ident.Options().LookupData(kLookupDataSonicData, true);
    ident.Options().LookupData(kLookupDataExternalIds, true);
//    ident.Options().LookupData(kLookupDataGlobalIds, true);
//    ident.Options().LookupData(kLookupDataAdditionalCredits, true);
//    ident.Options().LookupData(kLookupDataSortable, true);

    ident.AudioProcessStart(44100, 16, 2);
    ident.AudioProcess((const gnsdk_byte_t*) audio_chunk.c_str(), audio_chunk.length());
//    ident.AudioProcessStop();
//    ident.IdentifyAlbumAsync();
//    ident.WaitForIdentify(60000);
    ident.IdentifyAlbum();
//    ident.IdentifyCancel();

    // may throw GnError("GCSLGCSP         GCSP: Waveform Lookup error: [16] afp: Not enough time to join queue 'slave1'")
    // (randomly?)
//    stream_events.throw_if_error();

    if (stream_events.has_result()) {
        return py::dict("success"_a=true, "albums"_a=stream_events.get_result());
    } else {
        return py::dict("success"_a=false);
    }
}

PYBIND11_MODULE(gn, m) {
    m.def("recognize", &recognize, "user"_a, "audio_chunk"_a, "logger"_a);
    m.def("init_gracenote", &init_gracenote);
    m.def("enable_logging", &enable_logging, "filename"_a);
    m.def("create_or_load_user", &create_or_load_user, "client_id"_a, "client_tag"_a, "user_id"_a);

    py::class_<GnManager>(m, "GnManager");
    py::class_<GnUser>(m, "GnUser");

    static py::exception<GnError> gn_error(m, "GnError");
    py::register_exception_translator([](std::exception_ptr p) {
        try {
            if (p) std::rethrow_exception(p);
        } catch (const GnError &e) {
            gn_error((std::string(e.ErrorModule())  // GCSLDatatypes
                      + " " + std::string(e.ErrorAPI())      // gnsdk_manager_user_register
                      + " " + std::string(e.ErrorDescription())).c_str());  // User Create New: client id '1' not supported by current license
        }
    });
}
