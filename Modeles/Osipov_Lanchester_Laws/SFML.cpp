#include <iostream>
#include <SFML/Graphics.hpp>
#include<thread>
#include<time.h>
#include <ctime>
#include <ratio>
#include <chrono>

void TH_Delay(uint32_t ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void Delay(int32_t ms)
{
	if (ms <= 0) return;
	std::thread t(TH_Delay, ms);
	t.join();
}

struct Army
{
	float manpower = 0;
	float power = 1;

	void Calc(Army* enemy, float dt)
	{
		float otm = this->manpower;
		float oem = enemy->manpower;

		this->manpower = std::max(0.0f, this->manpower - enemy->power * enemy->manpower * dt);
		enemy->manpower = std::max(0.0f, enemy->manpower - this->power * this->manpower * dt);
	}


};

bool isWindowOpen = false;

Army A, B;

sf::Vertex lineA[500];
sf::Vertex lineB[500];

void Draw(sf::RenderWindow &window)
{
	for (int i = 0; i < 500; i++)
	{
		lineA[i].color = sf::Color::Green;
		lineB[i].color = sf::Color::Red;
	}


	window.draw(lineA, 500, sf::PrimitiveType::LinesStrip);
	window.draw(lineB, 500, sf::PrimitiveType::LinesStrip);
}

int main()
{
	int size = 1000;

	A.manpower = 1000;
	B.manpower = 500;
	B.power = 3.9921;

	float maxx = std::max(A.manpower, B.manpower);

	for (int i = 0; i < 500; i++)
	{
		lineA[i].position.x = 50 + i * (size - 100) / float(500);
		lineB[i].position.x = 50 + i * (size - 100) / float(500);
		lineA[i].position.y = 950 - (A.manpower/maxx) * 900;
		lineB[i].position.y = 950 - (B.manpower/maxx) * 900;
		for (int j = 0; j < 10; j++)
			A.Calc(&B, 0.001);



	}


	sf::ContextSettings setting;
	setting.antialiasingLevel = 4;

	sf::RenderWindow window(sf::VideoMode(size, size), "SFML",sf::Style::Default, setting);
	float fps = 1;
	double dt = 0;
	auto t1 = std::chrono::high_resolution_clock::now();
	auto t2 = std::chrono::high_resolution_clock::now();
	double adt = 1000/fps;
	int __GG = 0;


	isWindowOpen = true;
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();

		Draw(window);

		t2 = std::chrono::high_resolution_clock::now();
		double dt = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count() * 1000;
		t1 = std::chrono::high_resolution_clock::now();
		adt = (adt < 0 ? 4 : adt * 0.9995 + dt * 0.0005);
		std::cout << int(dt * 1000) << " " << int(adt * 1000) << " " << int(1000.0 / adt) << "        \r";

		Delay(std::max(1000 / fps - dt, 1.0));
		window.display();
		__GG++;
	}
	isWindowOpen = false;
	return 0;

}