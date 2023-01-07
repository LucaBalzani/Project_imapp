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
  for (int grain_size = 1; grain_size <= display_height; grain_size < 10 ? ++grain_size : grain_size += 10)
  {
    // Measure the time taken to process the image
    auto start = std::chrono::steady_clock::now();

    tbb::simple_partitioner partitioner{};

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
        },
        partitioner);

    auto end = std::chrono::steady_clock::now();
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
          },
          partitioner);
      image.saveToFile(namefile);
    }
  }
  image.saveToFile(static_cast<std::string>("Mandelbrot.png"));

  std::vector<int> grains;
  std::vector<double> times;
  std::ofstream out("Time_vs_grain_size.txt", std::ios::out);
  out << "Grain size and execution time obtained\n\nGrain size\tExecution time [ms]\n\n";
  for (auto const &[grain_size, elapsed_time] : elapsed_times)
  {
    grains.push_back(grain_size);
    times.push_back(elapsed_time / 1000.);
    out << grain_size << "\t\t" << elapsed_time / 1000. << '\n';
  }

  sf::Image image_time_grain;
  image_time_grain.create(display_width, display_height);

  // Set background colour to white
  for (int x = 0; x < display_width; ++x)
  {
    for (int y = 0; y < display_height; ++y)
    {
      image_time_grain.setPixel(x, y, sf::Color::White);
    }
  }

  // Determine the minimum and maximum values for the time and grain size data
  double timeMin = *std::min_element(times.begin(), times.end());
  double timeMax = *std::max_element(times.begin(), times.end());
  int grainMin = *std::min_element(grains.begin(), grains.end());
  int grainMax = *std::max_element(grains.begin(), grains.end());

  timeMin *= 0.95;
  timeMax *= 1.05;
  grainMin = -40;
  grainMax *= 1.05;

  const int radius = 3;
  // Loop through the data points and plot them on the image
  for (unsigned int i = 0; i < times.size(); ++i)
  {
    // Normalize the time and grain size values to the range [0, 1]
    double g = (times[i] - timeMin) / (timeMax - timeMin);
    double t = static_cast<double>((grains[i] - grainMin)) / (grainMax - grainMin);

    // Map the normalized values to the range [0, width] and [height, 0] respectively
    int x = static_cast<int>(t * display_width);
    int y = static_cast<int>(display_height - g * display_height);

    for (int dx = -radius; dx <= radius; ++dx)
    {
      for (int dy = -radius; dy <= radius; ++dy)
      {
        if (std::sqrt(dx * dx + dy * dy) <= radius && x + dx > 0 && y + dy > 0 && x + dx < display_width && y + dy < display_height)
        {
          image_time_grain.setPixel(x + dx, y + dy, sf::Color::Black);
        }
      }
    }
  }

  // Save the image to a file
  image_time_grain.saveToFile("Time_vs_grain_size.png");

  auto minimum_time = std::min_element(times.begin(), times.end());
  std::cout << "\nThe minimum time of " << times[std::distance(times.begin(), minimum_time)] << " ms corresponding to a grain size of " << grains[std::distance(times.begin(), minimum_time)] << ".\n\n";

  out << "\nThe minimum execution time (" << times[std::distance(times.begin(), minimum_time)] << " ms) corresponds to a grain size of " << grains[std::distance(times.begin(), minimum_time)] << ".\n";
  out.close();
}
