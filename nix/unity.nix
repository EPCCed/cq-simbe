{ lib, stdenv, unity-test, cmake, ninja }:
assert unity-test.version == "2.6.1";
stdenv.mkDerivation {
  pname = "unity-cmake";
  version = unity-test.version;
  src = unity-test.src;
  nativeBuildInputs = [ cmake ninja ];
  # Upstream CMake provides unity::framework. The PUBLIC definition is needed
  # in both unity.c and consuming tests to enable matching double assertions.
  postPatch = ''
    substituteInPlace CMakeLists.txt \
      --replace-fail 'add_library(''${PROJECT_NAME}::framework ALIAS ''${PROJECT_NAME})' \
        'add_library(''${PROJECT_NAME}::framework ALIAS ''${PROJECT_NAME})
    target_compile_definitions(''${PROJECT_NAME} PUBLIC UNITY_INCLUDE_DOUBLE)'
  '';
  cmakeFlags = [ "-DCMAKE_POSITION_INDEPENDENT_CODE=ON" ];
  meta = { license = lib.licenses.mit; platforms = lib.platforms.unix; };
}
