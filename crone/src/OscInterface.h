//
// Created by ezra on 11/4/18.
//

#ifndef SOFTCUT_OSCINTERFACE_H
#define SOFTCUT_OSCINTERFACE_H

#include <iostream>
#include <string>
#include <vector>

#include <lo/lo.h>


namespace crone {
    using std::string;

    class OscInterface {
    private:
        static lo_server_thread st;
        static bool quitFlag;
        static string port;

        // basically wrapper class for passing lambdas to OSC server thread
        class OscMethod {
            typedef void(*Handler)(lo_arg **argv, int argc);
            string path;
            string format;
        public:
            OscMethod(string p, string f, Handler h) : path(p), format(f), handler(h) {}
            Handler handler;
        };

        static std::vector<OscMethod> methods;

    private:

        typedef void(*Handler)(lo_arg **argv, int argc);
        static void handleLoError(int num, const char *m, const char *path) {
            std::cerr << "liblo error: " << num << "; " << m << "; " << path << std::endl;
        }

        static void addServerMethod(const char* path, const char* format, Handler handler) {
            OscMethod m(path, format, handler);
            methods.push_back(m);
            lo_server_thread_add_method(st, path, format,
            [] (const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *data) -> int
            {
                (void) path;
                (void) types;
                (void) msg;
                auto m = static_cast<OscMethod*>(data);
                m->handler(argv, argc);
                return 0;
            }, &methods.back());
        }

        static void addServerMethods();


    public:
        static void init() {
            quitFlag = false;
            port = "9999";
            st = lo_server_thread_new(port.c_str(), handleLoError);
            addServerMethods();
            lo_server_thread_start(st);
        }

        static bool shouldQuit() { return quitFlag; }

        static std::string getPortNumber() { return port; }
    };
}

#endif //SOFTCUT_OSCINTERFACE_H