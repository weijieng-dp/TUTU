/**___________________________________________________________________________/
@file          Errorlog.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

Errorlog implementation. std::any was alot more annoying than expected to work
with.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include "pch.h"
#include "Errorlog.h"

ErrorLog ErrorLog::instance; // Instance for singleton

#ifdef ERROR_SNAPSHOT
    // holy cungadero
    std::unordered_map<size_t, std::function<void(const std::any&)>> ErrorLog::AnyPrint{
        AnyPrintInit<int>([](int i)                     { std::cout << "Type: int, Value: " << i; }),
        AnyPrintInit<float>([](float i)                 { std::cout << "Type: float, Value: " << i; }),
        AnyPrintInit<double>([](double i)               { std::cout << "Type: double, Value: " << i; }),
        AnyPrintInit<char>([](char i)                   { std::cout << "Type: char, Value: " << i; }),
        AnyPrintInit<std::string>([](std::string i)     { std::cout << "Type: string, Value: " << i; }),
        AnyPrintInit<bool>([](bool i)                   { std::cout << "Type: bool, Value: " << i; }),
        AnyPrintInit<int*>([](int* i)                   { std::cout << "Type: int*, " << (i == nullptr ? "nullptr" : "not nullptr"); }),
        AnyPrintInit<float*>([](float* i)               { std::cout << "Type: float*, " << (i == nullptr ? "nullptr" : "not nullptr"); }),
        AnyPrintInit<double*>([](double* i)             { std::cout << "Type: double*, " << (i == nullptr ? "nullptr" : "not nullptr"); }),
        AnyPrintInit<char*>([](char* i)                 { std::cout << "Type: char*, " << (i == nullptr ? "nullptr" : "not nullptr"); }),
        AnyPrintInit<std::string*>([](std::string* i)   { std::cout << "Type: string*, " << (i == nullptr ? "nullptr" : "not nullptr"); }),
        AnyPrintInit<bool*>([](bool* i)                 { std::cout << "Type: bool*, " << (i == nullptr ? "nullptr" : "not nullptr"); })
    };
    std::unordered_map<size_t, std::function<std::any(const std::any&)>> ErrorLog::AnyDowncast{
        AnyDowncastInit<int*>([](int* i)                    -> std::any { if (i) return *i; else throw 0; }),
        AnyDowncastInit<float*>([](float* i)                -> std::any { if (i) return *i; else throw 0; }),
        AnyDowncastInit<double*>([](double* i)              -> std::any { if (i) return *i; else throw 0; }),
        AnyDowncastInit<char*>([](char* i)                  -> std::any { if (i) return *i; else throw 0; }),
        AnyDowncastInit<std::string*>([](std::string* i)    -> std::any { if (i) return *i; else throw 0; }),
        AnyDowncastInit<bool*>([](bool* i)                  -> std::any { if (i) return *i; else throw 0; }),
        AnyDowncastInit<int**>([](int** i)                  -> std::any { if (i) return *i; else throw 0; }),
        AnyDowncastInit<float**>([](float** i)              -> std::any { if (i) return *i; else throw 0; }),
        AnyDowncastInit<double**>([](double** i)            -> std::any { if (i) return *i; else throw 0; }),
        AnyDowncastInit<char**>([](char** i)                -> std::any { if (i) return *i; else throw 0; }),
        AnyDowncastInit<std::string**>([](std::string** i)  -> std::any { if (i) return *i; else throw 0; }),
        AnyDowncastInit<bool**>([](bool** i)                -> std::any { if (i) return *i; else throw 0; })
    };
#else
    std::unordered_map<size_t, std::function<void(const std::any&)>> ErrorLog::AnyPrint;
    std::unordered_map<size_t, std::function<std::any(const std::any&)>> ErrorLog::AnyDowncast;
#endif


ErrorLog& ErrorLog::Instance() { return instance; }

void ErrorLog::PrintLog() {
#ifndef ERROR_SNAPSHOT
    return; // Do nothing if not logging
#endif

    std::stringstream sstr;
    auto tempbuf = std::cout.rdbuf();
    bool android{ false };
#ifdef PLATFORM_ANDROID
    android = true;
#endif

    if (!outputfile.empty() || android) {
        // swapping cout and stringstream buffer
        std::cout.rdbuf(sstr.rdbuf());
    }

    // Print out "state" and "function" first
    std::cout << "In state: " << state << "\n";
    std::cout << "In function: " << function << "\n";

    for (std::pair<const std::string, std::any>& p : snapshot) {
        std::cout << "Variable: " << p.first << ", ";
        try {
            AnyPrint[hash(p.second)](p.second);
        }
        catch (...) {
            std::cout << "TYPE: " << p.second.type().name() << " IS NOT SUPPORTED FOR ERROR PRINTING\n";
        }
        std::cout << '\n';
    }

    if (!outputfile.empty() || android) {
        // swapping the buffers back
        std::cout.rdbuf(tempbuf);
#ifdef PLATFORM_ANDROID
        LOGE("[ENGINE]/[CRASH]");
        LOGE("%s", sstr.str().c_str());
        return;
#endif
        //CEO::Instance().GetManager<FileManager>()->EditorWriteFile(outputfile, sstr);
    }
    else if (outputfile.empty()) {
        std::cout << "Press any key to continue...\n";
        std::cin.get();
    }


}

void ErrorLog::SnapshotVariables(const std::string& s) {
#ifndef ERROR_SNAPSHOT
    return; // Do nothing if not logging
#endif

    if (storage.find(s) == storage.end()) return; // aka doesnt exist for some reason, stare

    std::map<std::string, std::any>& m = storage[s];
    snapshot.clear();

    for (std::pair<const std::string, std::any>& p : m) {
        if (AnyDowncast.find(hash(p.second)) == AnyDowncast.end()) {
            std::cout << "TYPE: " << p.second.type().name() << " IS NOT SUPPORTED FOR ERROR LOGGING\n";
            continue;
        }

        std::any a = AnyDowncast[hash(p.second)](p.second);
        snapshot[p.first] = a;

        // Seeing if it is a double pointer
        try {
            std::any a2 = AnyDowncast[hash(a)](a);
            std::string name = "*" + p.first;
            snapshot[name] = a2;

        }
        catch (...) {
            // do nothing
        }
    }
}

