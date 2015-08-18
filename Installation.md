# Using the build script #

Installation is similar to the one used in the VOTCA-CSG project. In addition to `tools` and `csg` you will need to install `kmc`, `moo`, and `ctp` modules. Ideally the following bash script should do the job for you:

```
prefix=~/votca
mkdir -p ${prefix}/src
cd ${prefix}/src
wget http://votca.googlecode.com/hg/build.sh
chmod +x build.sh
./build.sh --prefix ${prefix} --dev tools csg kmc moo ctp ctp-tutorials ctp-manual
```

If this does not work, please check the VOTCA-CSG [installation page](http://code.google.com/p/votca/wiki/Installing) for required dependencies and workarounds.

If you get `certificate error` when running `build.sh`, add the following section to your `.hgrc` file:
```
[alias]
clone = clone --insecure
```

# Running the program #

To run the program, source the VOTCARC file, i.e. in bash
```
source ~/votca/bin/VOTCARC.bash
```
In csh
```
source ~/votca/bin/VOTCARC.csh
```

# Dependencies #
The following packages should be installed (Ubuntu)
```
mercurial cmake g++ libboost-program-options-dev libboost-serialization-dev libboost-filesystem-dev libboost-timer-dev
libexpat-dev libfftw3-dev libgsl0-dev gromacs-dev libsqlite3-dev txt2tags
```

# Compiling the manual #
To compile the manual you will need additionally (Ubuntu)
```
xfig inkscape gsfonts-X11 ghostscript texlive texlive-latex-extra texlive-humanities
```

To get the pdf file, source the VOTCARC file and run `make` in the `ctp-manual` folder.

# Multiple Installations of VOTCA #

If you want to install more than one version of VOTCA, the following error might occur.

```
 ./votca_property: error while loading shared libraries: libboost_program_options.so.1.54.0: cannot open shared object file: No such file or directory
 make[2]: *** [src/tools/votca_property.man] Error 127
 make[1]: *** [src/tools/CMakeFiles/votca_property_manpage.dir/all] Error 2
```
to continue:
```
 cd tools
 cmake .
 make install
```