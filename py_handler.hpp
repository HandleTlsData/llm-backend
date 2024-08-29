#include "common.hpp"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

class py_handler
{
private:

public:
    py_handler();
    ~py_handler();

public:
    template<typename ReturnType>
    static ReturnType callPythonFunction(const std::string& moduleName, const std::string& functionName, const std::string& format, ...)
    {
        std::filesystem::path cwd = std::filesystem::current_path();
        std::string fullPath = cwd.string();

        std::string exec = "from " + moduleName + " import " + functionName + "\n";
        // + functionName + "()";

        PyRun_SimpleString(exec.c_str());

        PyObject* pModule = PyImport_ImportModule(moduleName.c_str());
        if (pModule == nullptr)
        {
            PyErr_Print();
            ERRLOG("unable to import module {}", moduleName);
            return static_cast<ReturnType>(NULL);
        }

        PyObject* pDict = PyModule_GetDict(pModule);
        if(pDict == NULL)
        {
            PyErr_Print();
            //ERRLOG("unable to get module dictionary {}: {0:x}", moduleName, (void*)pModule);
            return static_cast<ReturnType>(NULL);
        }

        PyObject* pFunc = PyDict_GetItemString(pDict, functionName.c_str());
        if (pFunc == NULL) 
        {
            /* PyDict_GetItemString does not set any error state so we have to. */
            PyErr_Format(PyExc_KeyError, "No \"%s\" in pickled dict.",
                            "someFunction");
            PyErr_Print();
            ERRLOG("unable to find function {} in {}", functionName, moduleName);
            return static_cast<ReturnType>(NULL);
        }

        PyObject* pArgs = {};
        PyObject* pReturn = {};

        if (PyCallable_Check(pFunc))
        {
            va_list vargs;
            va_start(vargs, format);
            pArgs=Py_VaBuildValue(format.c_str(), vargs);
            va_end(vargs);
            //pArgs=Py_BuildValue(format.c_str(), ...);
            PyErr_Print();
            LOG("calling py: {}->{}", moduleName, functionName);
            pReturn=PyObject_CallObject(pFunc,pArgs);
            PyErr_Print();
        } 
        else 
        {
            PyErr_Print();
        }

        if(pReturn == NULL)
        {
            PyErr_Print();
        }
        else
        {
            // std::cout << "Returned: ";
            if constexpr (std::is_arithmetic_v<ReturnType>) 
            {
                // std::cout << std::dec << PyLong_AsLong(pReturn) << std::endl;
                return ReturnType(PyLong_AsLong(pReturn));
            }  
            else if constexpr (std::is_same_v<ReturnType, std::string>) 
            {
                PyObject* pStr = PyUnicode_AsEncodedString(pReturn, "utf-8", "~E~");
                const char *bytes = PyBytes_AS_STRING(pStr);
                // std::cout << std::dec << bytes << std::endl;   
                return ReturnType(bytes);
            }
            else if constexpr (std::is_void_v<ReturnType>) 
            {
                return static_cast<ReturnType>(NULL);
            }
        } 
    }
};

inline const auto g_pyHandler = std::make_unique< py_handler >();