#include <SFML/Graphics.hpp>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>
#include <chrono>
#include <cassert>
#include <complex>
#include <iostream>
#include <string>
#include <thread>

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

auto to_color(int k, double opt = 0.0) /// MODIFICATA PER COLORARE DIVERSAMENTE
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

/*sf::Image painter(sf::RenderWindow& window, int display_width, int display_height){
  Complex const top_left{-2.2, 1.5};
  Complex const lower_right{0.8, -1.5};
  auto const diff = lower_right - top_left;

  auto const delta_x = diff.real() / display_width;
  auto const delta_y = diff.imag() / display_height;

  window.setSize(sf::Vector2u(display_width, display_height));

  sf::Image image;
  image.create(window.getSize().x, window.getSize().y);

  for (int row = 0; row != display_height; ++row)
  {
    for (int column = 0; column != display_width; ++column)
    {
      auto k = mandelbrot(top_left + Complex{delta_x * column, delta_y * row});
      image.setPixel(column, row, to_color(k));
    }
  }

  return image;
}*/

/*class Rendering
{
  int m_w;
  int m_h;
  std::vector<sf::Color> m_colors;

public:
  Rendering(int w, int h) : m_w{w}, m_h{h}, m_colors(w * h)
  {
  }
  sf::Color const &at(int r, int c) const
  {
    assert(r >= 0 && r < m_h);
    assert(c >= 0 && c < m_w);
    return m_colors[r * m_w + c];
  }
  sf::Color &at(int r, int c)
  {
    assert(r >= 0 && r < m_h);
    assert(c >= 0 && c < m_w);
    return m_colors[r * m_w + c];
  }
};*/

int main()
{
  int const display_width{600};
  int const display_height = display_width; // MODIFICATO ERA int const display_height{600};

  Complex const top_left{-2.2, 1.5};
  Complex const lower_right{0.8, -1.5};
  auto const diff = lower_right - top_left;

  auto const delta_x = diff.real() / display_width;
  auto const delta_y = diff.imag() / display_height;

  sf::RenderWindow window(sf::VideoMode(display_width, display_height),
                          "Mandelbrot Set");
  window.setFramerateLimit(60);

  // sf::RenderWindow window_grain(sf::VideoMode(display_width, display_height), "Time vs Grain Size");

  std::vector<std::pair<int, double>> elapsed_times;

  // Rendering rendering(display_width, display_height);

  //sf::RectangleShape point{sf::Vector2f{1.f, 1.f}};

  ////ORIGINAL PART
  sf::Image image;
  image.create(window.getSize().x, window.getSize().y);
  sf::Texture texture;
  sf::Sprite sprite;

  // Vary the grain size of the parallel_for loop
  for (int grain_size = 1; grain_size <= display_height / 10; ++grain_size)
  {
    // Measure the time taken to process the image
    auto start = std::chrono::high_resolution_clock::now();

    tbb::parallel_for(
        tbb::blocked_range2d<int>(0, display_height, grain_size * 10, 0, display_width, grain_size * 10),
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
        });

    /*// Use tbb::parallel_for to process each row in parallel
    tbb::parallel_for(0, display_height, grain_size, [&](int row) {
      tbb::parallel_for(0, display_width, grain_size, [&](int column){
        auto k = mandelbrot(top_left + Complex{delta_x * column, delta_y * row});
        image.setPixel(column, row, to_color(k));
      });
      //for (int column = 0; column != display_width; ++column) {
      //  auto k = mandelbrot(top_left + Complex{delta_x * column, delta_y * row});
      //  rendering.at(row, column) = to_color(k);
      //}
    });*/

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    elapsed_times.emplace_back(grain_size, elapsed_time);

    std::cout << "Grain size: " << grain_size * 10 << ", elapsed time: " << elapsed_time << " microseconds" << std::endl;

    window.clear();

    /*for (int row = 0; row != display_height; ++row) {
      for (int column = 0; column != display_width; ++column) {
        point.setPosition(column, row);
        point.setFillColor(rendering.at(row, column));
        window.draw(point);
      }
    }*/
    texture.loadFromImage(image);
    sprite.setTexture(texture);

    window.draw(sprite);

    window.display();

    if (grain_size != 60 && !(grain_size % 20))
    {
      std::string namefile = std::string("Mandelbrot_at_") + std::to_string(grain_size) + std::string(".png");
      auto color = grain_size / 20.0;
      tbb::parallel_for(
          tbb::blocked_range2d<int>(0, display_height, grain_size * 10, 0, display_width, grain_size * 10),
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
      window.clear();
      texture.loadFromImage(image);
      sprite.setTexture(texture);
      window.draw(sprite);
      window.display();
      image.saveToFile(namefile);
    }

    // using namespace std::chrono_literals;
    // std::this_thread::sleep_for(1000ms);
  }

  std::vector<int> grains;
  std::vector<double> times;
  for (auto const& [grain_size, elapsed_time] : elapsed_times) {
    grains.push_back(grain_size*10);
    times.push_back(elapsed_time/1000.);
  }

  // Plot the data using matplotlib-cpp
  plt::plot(grains, times);
  plt::xlabel("Grain size");
  plt::ylabel("Time [ms]");
  plt::grid(true);
  plt::save("Time_vs_grain_size.png");
  //plt::show();
  
  /*sf::RenderWindow window_grain(sf::VideoMode(601, 601), "Time vs Grain Size");
  sf::Image graph;
  graph.create(601, 601, sf::Color::White);

  double maximum_time = 0.0;

  for (auto const& [grain, time] : elapsed_times)
  {
    if (maximum_time < time) maximum_time = time;
  }

  window_grain.clear();
  for (auto const& [grain, time] : elapsed_times)
  {
    sf::Vertex line[] = {
          sf::Vertex({static_cast<float>(grain), 0.f}, sf::Color::Black),
          sf::Vertex({static_cast<float>(grain), static_cast<float>(time * 1000)}, sf::Color::Black),
      };
      window_grain.draw(line, 2, sf::Lines);
    //graph.setPixel(grain, time*600/maximum_time, sf::Color(0, 0, 0));
  }
  sf::Texture texture_grain;
  texture_grain.loadFromImage(graph);
  sf::Sprite sprite_grain;
  sprite_grain.setTexture(texture_grain);
  window_grain.draw(sprite_grain);
  window_grain.display();*/

  /*                                                        /////ORIGINAL PART
  for (int row = 0; row != display_height; ++row)
  {
    for (int column = 0; column != display_width; ++column)
    {
      auto k = mandelbrot(top_left + Complex{delta_x * column, delta_y * row});
      image.setPixel(column, row, to_color(k));
    }
  }

  sf::Texture texture;
  texture.loadFromImage(image);
  sf::Sprite sprite;
  sprite.setTexture(texture);*/

  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
        window.close();
      if (event.type == sf::Event::KeyPressed)
      { ///////////////////////////////////////////////
        if (event.key.code == sf::Keyboard::P)
        {                              ////////////////////////////
          std::string output_file{""}; /////////////////////////////////////
          std::cout << "Please insert the name of the png file where you want to save the image: ";
          std::cin >> output_file;
          if (output_file.size() > 5)
          {
            if (!output_file.compare(output_file.size() - 4, 4, static_cast<std::string>(".png")))
            {
              image.saveToFile(output_file);
            }
            else
            {
              output_file += static_cast<std::string>(".png");
              image.saveToFile(output_file);
            }
          }
          else
          {
            output_file += static_cast<std::string>(".png");
            image.saveToFile(output_file);
          }
          std::cout << "Image saved as \"" << output_file << "\"." << std::endl;
        }
      }
    }

    window.setKeyRepeatEnabled(false); //////////////////////////////////////

    window.clear();

    window.draw(sprite);

    window.display();

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(15ms);
  }
}
