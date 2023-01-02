#include <SFML/Graphics.hpp>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>
#include <chrono>
#include <cassert>
#include <complex>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>

#include <matplotlibcpp.h>
namespace plt = matplotlibcpp;

using Complex = std::complex<double>;

int mandelbrot(Complex const &c)
{
  int i = 0;
  auto z = c;
  for (; i != 256 && norm(z) < 4.; ++i)
  {
    z = z * z + c;
  }
  return i;
}

auto to_color(int k, double opt = 0.0) 
{
  if (!opt)
  {
    return k < 256 ? sf::Color{static_cast<sf::Uint8>(10 * k), 0, 0} : sf::Color::Black;
  }
  else
  {
    if (opt == 1.0)
      return k < 256 ? sf::Color{0, static_cast<sf::Uint8>(10 * k), 0} : sf::Color::Black;
    else
      return k < 256 ? sf::Color{0, 0, static_cast<sf::Uint8>(10 * k)} : sf::Color::Black;
  }
}

int main()
{
  int const display_width{800};
  int const display_height = display_width;

  Complex const top_left{-2.2, 1.5};
  Complex const lower_right{0.8, -1.5};
  auto const diff = lower_right - top_left;

  auto const delta_x = diff.real() / display_width;
  auto const delta_y = diff.imag() / display_height;

  std::vector<std::pair<int, double>> elapsed_times;

  sf::Image image;
  image.create(display_width, display_height);

  // Vary the grain size of the parallel_for loop
  for (int grain_size = 1; grain_size <= display_height; grain_size < 10 ? ++grain_size : grain_size+=10 )
  {
    // Measure the time taken to process the image
    auto start = std::chrono::high_resolution_clock::now();

    tbb::parallel_for(
        tbb::blocked_range2d<int>(0, display_height, grain_size, 0, display_width, grain_size),
        [&](const tbb::blocked_range2d<int> &fragment)
        {
          for (int row = fragment.rows().begin(); row != fragment.rows().end(); ++row)
          {
            for (int column = fragment.cols().begin(); column != fragment.cols().end(); ++column)
            {
              auto k = mandelbrot(top_left + Complex{delta_x * column, delta_y * row});
              image.setPixel(column, row, to_color(k));
            }
          }
        }); //By default a simple_partitioner is used

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    elapsed_times.emplace_back(grain_size, elapsed_time);

    std::cout << "Grain size: " << grain_size << ", elapsed time: " << elapsed_time << " microseconds" << std::endl;

    if (grain_size != display_height && !(grain_size % 300))
    {
      std::string namefile = std::string("Mandelbrot_at_") + std::to_string(grain_size) + std::string(".png");
      auto color = grain_size / 300.0;
      tbb::parallel_for(
          tbb::blocked_range2d<int>(0, display_height, grain_size, 0, display_width, grain_size),
          [&](const tbb::blocked_range2d<int> &fragment)
          {
            for (int row = fragment.rows().begin(); row != fragment.rows().end(); ++row)
            {
              for (int column = fragment.cols().begin(); column != fragment.cols().end(); ++column)
              {
                auto k = mandelbrot(top_left + Complex{delta_x * column, delta_y * row});
                image.setPixel(column, row, to_color(k, color));
              }
            }
          });
      image.saveToFile(namefile);
    }
  }
  image.saveToFile(static_cast<std::string>("Mandelbrot.png"));

  std::vector<int> grains;
  std::vector<double> times;
  std::ofstream out("Time_vs_grain_size.txt", std::ios::out);
  out<<"Grain size and execution time obtained\n\nGrain size\tExecution time [ms]\n\n";
  for (auto const& [grain_size, elapsed_time] : elapsed_times) {
    grains.push_back(grain_size);
    times.push_back(elapsed_time/1000.);
    out<<grain_size<<"\t\t"<<elapsed_time/1000.<<'\n';
  }

  // Plot the data using matplotlib-cpp
  plt::figure();
  plt::plot(grains, times);
  plt::xlabel("Grain size");
  plt::ylabel("Time [ms]");
  plt::title("Time vs grain size");
  plt::grid(true);
  plt::save("Time_vs_grain_size.png");

  auto minimum_time = std::min_element(times.begin(),times.end());
  std::cout<<"\nThe minimum time of "<<times[std::distance(times.begin(), minimum_time)]<<" ms corresponding to a grain size of "<<grains[std::distance(times.begin(), minimum_time)]<<".\n\n";

  out<<"\nThe minimum execution time ("<<times[std::distance(times.begin(), minimum_time)]<<" ms) corresponds to a grain size of "<<grains[std::distance(times.begin(), minimum_time)]<<".\n";
  out.close();

}
