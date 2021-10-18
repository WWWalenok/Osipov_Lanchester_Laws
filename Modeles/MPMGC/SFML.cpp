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



// MPMGC - Multiparametric model of group combat

struct Army
{
	struct Division
	{

		static void No_move(Division& select, Army& F, Army& E, double dt)
		{

		}

		static double Static_weapon_precision(Division& select, Division& enemy)
		{
			return select.base_weapon_precision;
		}

		static double Static_chance_of_fier(Division& select, Division& enemy)
		{
			return 1;
		}

		static double Area_proporcional_chance_of_fier(Division& select, Division& enemy)
		{
			return enemy.area;
		}

		static double Distance_proporcional_chance_of_fier(Division& select, Division& enemy)
		{
			return 1 / (0.0001 + abs(select.position - enemy.position));
		}

		// Static parametrs
		double
			damage = 1, //damage
			fier_cd = 1,
			area_of_effect = 0,
			damage_penetration = 0,
			deffenc = 0,
			durability = 1
			;
		// Dinamic parrametrs
		double
			position = 0,
			area = 1,
			evasion = 0,
			man_power = 1,
			resource_power = 1,
			mind_power = 1,
			salvoCd = 1;
		//Other parametrs
		double
			base_weapon_precision = 1;

		// Func
		double (*weapon_precision)(Division& select, Division& enemy) = Static_weapon_precision;
		void (*move)(Division& select, Army& F, Army& E, double dt) = No_move;
		double (*chance_of_fier)(Division& select, Division& enemy) = Static_chance_of_fier;
	};


	Division* divs;
	int div_count;
	void move(Army& E, double dt)
	{
		for (int i = 0; i < div_count; i++)
			divs[i].move(divs[i], *this, E, dt);
	}

	static double P(double p, double e)
	{
		if (p == 0)
			return 0.5 * (1 - e / (10 + e));
		double ep = exp2(-p);
		const double ln2 = log(2);
		return (1 / (1 - ep) + 1 / (-p * ln2)) * (1 - e / (10 + e));
	}

	static void CalcDamage(double summ_chance, double salvoCount, Division& At, Division& At_d, Division& De, Division& De_d)
	{
		float K = At.chance_of_fier(At, De) / summ_chance;

		double
			clear_damage = K * salvoCount * At.damage / De.durability * At.man_power * P(At.weapon_precision(At, De), De.evasion),
			aoe_coof = 1 + De.man_power * std::min(1.0, At.area_of_effect / De.area),
			armore_coof = 1 / (1 + exp(De.deffenc - At.damage_penetration)),
			resource_damage_coof = 1 / (1 + exp(10 * (De.deffenc - At.damage_penetration)));

		if (At_d.fier_cd > 0.5)
			std::cout << "";

		De_d.man_power = std::max(0.0, De_d.man_power - clear_damage * armore_coof * aoe_coof);

		De_d.area = std::max(0.0, De_d.area * De_d.man_power / De.man_power);

		De_d.mind_power = std::max(0.0, De_d.mind_power - clear_damage * aoe_coof * 2);

		De_d.resource_power = std::max(0.0, De_d.mind_power - clear_damage * resource_damage_coof * aoe_coof * 0.5);
	}

	static void Calc(Army& A, Army& B, double dt)
	{
#define __T

		Division* A_divs = new Division[A.div_count];
		Division* B_divs = new Division[B.div_count];

		for (int i = 0; i < A.div_count; i++)
			A_divs[i] = Division(A.divs[i]);
		for (int i = 0; i < B.div_count; i++)
			B_divs[i] = Division(B.divs[i]);

		for (int i = 0; i < A.div_count; i++) if (A_divs[i].man_power != 0)
		{
			Division
				& At = A_divs[i],
				& At_d = A.divs[i];

			float salvoCount = 0;
			At_d.salvoCd -= dt;
			while (At_d.salvoCd <= 0)
			{
				salvoCount += 1;
				At_d.salvoCd += At_d.fier_cd;
			}

			if (salvoCount == 0)continue;

			float summ_chance = 0;
			for (int j = 0; j < B.div_count; j++)
				if (B_divs[j].man_power != 0)
					summ_chance += A_divs[i].chance_of_fier(A_divs[i], B_divs[j]);

			for (int j = 0; j < B.div_count; j++)
				if (B_divs[j].man_power != 0)
				{
					Division
						& De = B_divs[j],
						& De_d = B.divs[j];

					CalcDamage(summ_chance, salvoCount, At, At_d, De, De_d);
				}
		}

		for (int i = 0; i < B.div_count; i++) if (B_divs[i].man_power != 0)
		{
			Division
				& At = B_divs[i],
				& At_d = B.divs[i];

			float salvoCount = 0;
			At_d.salvoCd -= dt;
			while (At_d.salvoCd <= 0)
			{
				salvoCount += 1;
				At_d.salvoCd += At_d.fier_cd;
			}

			if (salvoCount == 0)continue;

			float summ_chance = 0;
			for (int j = 0; j < A.div_count; j++)
				if (A_divs[j].man_power != 0)
					summ_chance += B_divs[i].chance_of_fier(B_divs[i], A_divs[j]);

			for (int j = 0; j < A.div_count; j++)
				if (A_divs[j].man_power != 0)
				{
					Division
						& De = A_divs[j],
						& De_d = A.divs[j];

					CalcDamage(summ_chance, salvoCount, At, At_d, De, De_d);
				}
		}


		A.move(B, dt);
		B.move(A, dt);

		delete[] A_divs;
		delete[] B_divs;
	}
};

Army A, B;

Army::Division
Infantry
{
	0.01,
	0.0082242,
	0,
	0,
	1,
	1,
	//
	0,
	1,
	0,
	0,
	0,
	0,
	0,
	//
	-2,
	//
	Army::Division::Static_weapon_precision,
	Army::Division::No_move,
	Army::Division::Distance_proporcional_chance_of_fier
},
Artelery
{
	0.1,
	0.5,
	1,
	2,
	0,
	10,
	//
	0,
	1,
	3,
	0,
	0,
	0,
	2,
	//
	0,
	//
	Army::Division::Static_weapon_precision,
	Army::Division::No_move,
	Army::Division::Area_proporcional_chance_of_fier
};

sf::Vertex lineA_i[500];
sf::Vertex lineB_i[500];
sf::Vertex lineA_a[500];
sf::Vertex lineB_a[500];


void Draw(sf::RenderWindow& window)
{
	window.draw(lineA_i, 500, sf::PrimitiveType::LinesStrip);
	window.draw(lineB_i, 500, sf::PrimitiveType::LinesStrip);
	window.draw(lineA_a, 500, sf::PrimitiveType::LinesStrip);
	window.draw(lineB_a, 500, sf::PrimitiveType::LinesStrip);
}


void main()
{
	for (int i = 0; i < 500; i++)
	{
		lineA_i[i].color = sf::Color::Green;
		lineB_i[i].color = sf::Color::Red;
		lineA_a[i].color = sf::Color(128, 255, 128);
		lineB_a[i].color = sf::Color(255, 128, 128);
	}

	A.divs = new Army::Division[2]
	{
		Army::Division(Infantry),
		Army::Division(Artelery)
	};
	A.div_count = 2;

	B.divs = new Army::Division[2]
	{
		Army::Division(Infantry),
		Army::Division(Artelery)
	};
	B.div_count = 2;

	Army::Division
		& A_i = A.divs[0],
		& A_a = A.divs[1],
		& B_i = B.divs[0],
		& B_a = B.divs[1];


	A_i.man_power = 3000;
	A_i.position = 1;
	A_i.area = A_i.man_power * 0.025;

	A_a.man_power = 200;
	A_a.position = 100;
	A_a.area = A_a.man_power * 0.1;

	B_i.man_power = 4000;
	B_i.position = -1;
	B_i.area = B_i.man_power * 0.025;

	B_a.man_power = 100;
	B_a.position = -100;
	B_a.area = B_a.man_power * 0.1;

	int size = 1000;

	float maxArea =
		std::max(std::max(std::max(A_i.area, B_i.area), A_a.area), B_a.area);

	for (int i = 0; i < 500; i++)
	{
		lineA_i[i].position.x = 50 + i * (size - 100) / float(500);
		lineB_i[i].position.x = 50 + i * (size - 100) / float(500);
		lineA_a[i].position.x = 50 + i * (size - 100) / float(500);
		lineB_a[i].position.x = 50 + i * (size - 100) / float(500);

		lineA_i[i].position.y = 950 - (A_i.area / maxArea) * 900;
		lineB_i[i].position.y = 950 - (B_i.area / maxArea) * 900;
		lineA_a[i].position.y = 950 - (A_a.area / maxArea) * 900;
		lineB_a[i].position.y = 950 - (B_a.area / maxArea) * 900;

		for (int t = 0; t < 1; t++)
			Army::Calc(A, B, 0.01);



	}


	sf::ContextSettings setting;
	setting.antialiasingLevel = 4;

	sf::RenderWindow window(sf::VideoMode(size, size), "SFML", sf::Style::Default, setting);
	float fps = 60;
	double dt = 0;
	auto t1 = std::chrono::high_resolution_clock::now();
	auto t2 = std::chrono::high_resolution_clock::now();
	double adt = 1000 / fps;
	int __GG = 0;


	bool isWindowOpen = true;
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
		for (int j = 0; j < 1; j++)
		{
			for (int i = 0; i < 500 - 1; i++)
			{
				lineA_i[i].position.y = lineA_i[i + 1].position.y;
				lineA_a[i].position.y = lineA_a[i + 1].position.y;
				lineB_i[i].position.y = lineB_i[i + 1].position.y;
				lineB_a[i].position.y = lineB_a[i + 1].position.y;
			}
			int i = 499;
			lineA_i[i].position.y = 950 - (A_i.area / maxArea) * 900;
			lineB_i[i].position.y = 950 - (B_i.area / maxArea) * 900;

			lineA_a[i].position.y = 950 - (A_a.area / maxArea) * 900;
			lineB_a[i].position.y = 950 - (B_a.area / maxArea) * 900;

			for (int t = 0; t < 1; t++)
				Army::Calc(A, B, 0.01);
		}

	}
	isWindowOpen = false;


}