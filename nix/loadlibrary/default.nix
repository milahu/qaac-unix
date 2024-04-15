# based on nixpkgs/pkgs/tools/misc/loadlibrary/default.nix

{ lib
, cabextract
, fetchFromGitHub
, fetchurl
, readline
, stdenv_32bit
, gawk
, binutils
}:

# stdenv_32bit is needed because the program depends upon 32-bit libraries and does not have
# support for 64-bit yet: it requires libc6-dev:i386, libreadline-dev:i386.

stdenv_32bit.mkDerivation rec {
  pname = "loadlibrary";
  version = "unstable-2022-01-23";
  src = fetchFromGitHub {
    owner = "taviso";
    repo = "loadlibrary";
    rev = "c40033b12fd82514f8ee4e42a25906b773b91b5c";
    hash = "sha256-AQyiFZEPFKDCQ9LUK/AHi3wA+I23XPEe3wBIudvxOY4=";
  };

  # no. this url is not stable
  /*
  src-mpam-exe = fetchurl {
    # loadlibrary/engine/README.md
    url = "http://go.microsoft.com/fwlink/?LinkID=121721&arch=x86";
    name = "mpam-fe.exe";
    hash = "sha256-Gfz8RUkTvKvO2GJuhHcEVm3U7SwXg+sL81J2uI5NiGA=";
  };
  */

  nativeBuildInputs = [
    cabextract
  ];

  buildInputs = [
    #readline
    gawk # awk
    binutils # as
  ];

  /*
  postUnpack = ''
    pushd $sourceRoot >/dev/null
    cabextract ${src-mpam-exe}
    popd >/dev/null
  '';
  */

  /*
    substituteInPlace mpscript.c mpclient.c \
      --replace-fail '"engine/mpengine.dll"' "\"$out/opt/loadlibrary/example/mpclient/mpengine.dll\""
  */
  patchPhase = ''
    substituteInPlace genmapsym.sh \
      --replace-fail 'awk ' '${gawk}/bin/awk ' \
      --replace-fail ' as -o ' ' ${binutils}/bin/as -o ' \

    cat >mpengine.dll.fetch.sh <<EOF
    #!/bin/sh
    # this url is not stable, so mpengine.dll is not part of this package
    # mpclient is only an example app
    curl -L -o mpam-fe.exe "http://go.microsoft.com/fwlink/?LinkID=121721&arch=x86"
    cabextract mpam-fe.exe
    stat mpengine.dll
    EOF
    chmod +x mpengine.dll.fetch.sh
  '';

  #  cp -v -t $out/opt/loadlibrary/example/mpclient mpclient.c mpclient mpengine.dll
  installPhase = ''
    mkdir -p $out/bin
    cp -v genmapsym.sh $out/bin

    mkdir -p $out/opt/loadlibrary/example/mpclient
    cp -v -t $out/opt/loadlibrary/example/mpclient mpclient.c mpclient mpengine.dll.fetch.sh

    mkdir -p $out/lib
    cp -v -t $out/lib peloader/libpeloader.a intercept/libdisasm.a

    mkdir -p $out/include/loadlibrary
    cp -v -t $out/include/loadlibrary peloader/*.h include/*.h intercept/hook.h peloader/winapi/rootcert.h
  '';

  meta = with lib; {
    homepage = "https://github.com/taviso/loadlibrary";
    description = "Porting Windows Dynamic Link Libraries to Linux";
    platforms = platforms.linux;
    maintainers = [ ];
    license = licenses.gpl2;
    mainProgram = "mpclient";
  };
}
