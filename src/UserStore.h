#ifndef CMDLINE_MUSICDETECTION_USERSTORE_H
#define CMDLINE_MUSICDETECTION_USERSTORE_H

#include <pybind11/pybind11.h>
#include <gnsdk.hpp>
#include <string>
#include <fstream>

using gracenote::IGnUserStore;
using gracenote::GnString;

namespace py = pybind11;


class UserStore : public IGnUserStore
{
private:
    int user_id;

public:
    UserStore (int user_id_) : user_id(user_id_) {}

    GnString LoadSerializedUser(gnsdk_cstr_t client_id) override
    {
        std::string filename = "users/" + std::string(client_id) + "_" + std::to_string(user_id) + ".txt";

        std::fstream user_reg_file(filename, std::ios_base::in);

//        py::print("load:", !user_reg_file.fail());

        GnString user_data;
        if(!user_reg_file.fail())
        {
            std::string serialized;
            user_reg_file >> serialized;
            user_data = serialized.c_str();
        }

        return user_data;
    }

    bool StoreSerializedUser(gnsdk_cstr_t client_id, gnsdk_cstr_t user_data) override
    {
        std::string filename = "users/" + std::string(client_id) + "_" + std::to_string(user_id) + ".txt";

        std::fstream user_reg_file(filename, std::ios_base::out);

//        py::print("save:", !user_reg_file.fail());

        if(!user_reg_file.fail())
        {
            user_reg_file << user_data;
            return true;
        }

        return false;
    }
};


#endif //CMDLINE_MUSICDETECTION_USERSTORE_H
