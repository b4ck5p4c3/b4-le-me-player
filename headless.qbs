import qbs

Project {
    references: ["playback.qbs"]
    CppApplication {
        name: "leme-headless"
        consoleApplication: true
        Depends { name: "Qt"; submodules: ["core", "mqtt", "qml"] }
        Depends { name: "leme-playback" }
        cpp.cxxLanguageVersion: "c++23"
        cpp.defines: ["QT_NO_KEYWORDS"]
        files: ["src/headless/main.cpp"]
    }
    CppApplication {
        name: "test-headless"
        consoleApplication: true
        Depends { name: "Qt"; submodules: ["core", "testlib", "network", "qml"] }
        Depends { name: "leme-playback" }
        cpp.cxxLanguageVersion: "c++23"
        cpp.defines: ["QT_NO_KEYWORDS"]
        files: ["src/headless/tests/test_player.cpp"]
    }
}
