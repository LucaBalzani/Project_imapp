# Project Computer Scince for High Energy Physics
This program is able to compute and graphicly represent the Mandelbrot set.

The main objective of the code is to parallelize the computations needed to obtain the complex points belonging to the Mandelbrot set.

In order to achieve this a serie of 'tbb::parallel_for' are used, along with other structures from tbb.