import qbs

StaticLibrary {
    name: "leme-playback"
    Depends { name: "Qt.core" }
    cpp.cxxLanguageVersion: "c++23"
    cpp.defines: ["QT_NO_KEYWORDS"]
    files: ["src/headless/player.cpp", "src/headless/player.h", "src/headless/volume.h"]
    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt.core" }
        cpp.includePaths: [exportingProduct.sourceDirectory + "/src"]
        cpp.dynamicLibraries: ["mpv"]
    }
}
