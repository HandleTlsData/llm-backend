#include "py_handler.hpp"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

py_handler::py_handler()
{
    PyStatus status;

    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    config._init_main = 0;

    status = Py_InitializeFromConfig(&config);
    PyConfig_Clear(&config);
    if (PyStatus_Exception(status)) {
        Py_ExitStatusException(status);
    }

    status = _Py_InitializeMain();
    if (PyStatus_Exception(status)) {
        Py_ExitStatusException(status);
    }

    //more module locations in future
    std::filesystem::path cwd = std::filesystem::current_path() / "music";
    std::string fullPath = cwd.string();

    std::string exec = "import sys\nsys.path.append('" + fullPath + "')";
    PyRun_SimpleString(exec.c_str());
    LOG("PyHandler initialized");
}

py_handler::~py_handler()
{
    Py_Finalize();
}
