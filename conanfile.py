from conans import ConanFile, CMake


class CacheConan(ConanFile):
    name = "cache"
    version = "0.0.2"
    license = "BSD 3-Clause License"
    author = "Vladimir Petrigo <And your email here>"
    url = "https://github.com/vpetrigo/caches"
    description = "C++ LRU/FIFO/LFU Cache implementation"
    topics = ("header-only", "cache", "lru", "fifo", "lfu")
    exports_sources = "include/*", "CMakeLists.txt", "deps/*", "src/*", "test/*"
    no_copy_source = True
    settings = "os", "compiler", "build_type", "arch"
    # No settings/options are necessary, this is header only

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        self.run(f"ctest -C {self.settings.build_type} -V")

    def package(self):
        self.copy("*.hpp")

    def package_id(self):
        self.info.header_only()
