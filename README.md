# Project Computer Scince for High Energy Physics
This program is able to compute and graphicly represent the Mandelbrot set.

The main objective of the code is to parallelize the computations needed to obtain the complex points belonging to the Mandelbrot set.

In order to achieve this the `tbb::parallel_for` function is used, along with other structures from tbb (such as `tbb::blocked_range2d`, which allows to divide the two dimensional range of the image into subsets which can be dealt with separatly).

A deeper study on the efficiency of the computation of the Mandelbrot set is also carried out by varying the grain size.
In particular re-computing the Mandelbrot set dividing the complex plain which is considered in a changing number and size of slices.

The code is contained in the file *main.cpp* while the building options are in the *CMakeLists.txt* file, the file *project.txt* contains the text of the assignement.

This code has been put on this virtual machine and can be executed by running the docker image `immagine_running`.

The images are built via a Dockerfile which includes the instructions to install the packages needed to compile and execute the code, along with the creation of a directory required as volume which will then be linked to a local directory.

An example of the Dockerfile used to build the images is the following:

    FROM ubuntu:22.04

    RUN apt update && DEBIAN_FRONTEND=noninteractive apt install -y \
        g++ \
        cmake \
        libsfml-dev \
        libtbb-dev \
        git-all \

    RUN mkdir -p /workspace
    VOLUME /workspace

    WORKDIR /workspace


The libraries **g++** and **cmake** are used to compile and build the project.
The libraries libsfml-dev and libtbb-dev are needed to acces the graphical tools of __SFML__ and the parallelization functions of **TBB**.
The git toolkit is installed to clone the directory https://github.com/LucaBalzani/Project_imapp.git containing the project.

When creating the image used for running the program the libraries **g++**, **cmake** and **git** may not be installed, as they are not used. 

## To run the code
To run the code, on the proper environment, one should just build the code:

    cmake -S . -B build_release -DCMAKE_BUILD_TYPE=Release
    cmake --build build_release
    build_release/mandelbrot

The output of the code is then saved in the current directory.
The output includes 4 _png_ files and a _txt_ file:
- Mandelbrot.png ⟶ Representation of the Mandelbrot set in red (latest grain size tested, 800 pixels),
- Mandelbrot_at_300.png ⟶ Representation of the Mandelbrot set in green (grain size 300 pixels),
- Mandelbrot_at_600.png ⟶ Representation of the Mandelbrot set in blue (grain size 600 pixels),
- Time_vs_grain_size.png ⟶ Graphical representation of the elapsed time as a function of the grain size considered,
- Time_vs_grain_size.txt ⟶ Elapsed times for the various grain sizes used.