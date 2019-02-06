#include "MazeGeneratePNG.h"

MazeGeneratePNG::MazeGeneratePNG(int height, int width)
{
	// height and width must be odd numbers for the maze to be "correct"
	if ((!(height % 2)) || (!(width % 2)))
	{
		throw std::invalid_argument("Height and width must be odd numbers!");
	}

	this->height = height;
	this->width = width;

	// create a grid with only walls
	grid.resize(height);
	for (int y = 0; y < height; y++)
	{
		grid[y].resize(width);
		for (int x = 0; x < width; x++)
		{
			grid[y][x] = wall;
		}
	}
}

void MazeGeneratePNG::set_cell(int y, int x, int value)
{
	if (((int)grid.size() <= y) || ((int)grid[y].size() <= x))
	{
		throw std::invalid_argument("Out of boundary!");
	}
	else if ((wall != value) && (path != value))
	{
		throw std::invalid_argument("The input can only contain 0 or 1.");
	}
	else
	{
		grid[y][x] = value;
	}
}

int MazeGeneratePNG::get_cell(int y, int x)
{
	return grid[y][x];
}

void MazeGeneratePNG::generate(void)
{
	std::mt19937 random_generator(random_device());

	// set up sets
	sets.resize(grid.size());
	for (int y = 0; y < (int)grid.size(); y++)
	{
		sets[y].resize(grid[y].size());
	}

	// save every y, x coordinate with a possible movement (vertical or horizontal) and fill the sets with different values
	std::vector<element> elements;
	int i = 1;
	for (int y = 1; y < ((int)sets.size() - 1); y += 2)
	{
		for (int x = 1; x < ((int)sets[0].size() - 1); x += 2)
		{
			if ((y + 2) < ((int)sets.size() - 1))
			{
				elements.push_back({ y, x, vertical });
			}
			if ((x + 2) < ((int)sets[0].size() - 1))
			{
				elements.push_back({ y, x, horizontal });
			}
			sets[y][x] = i;
			i++;
		}
	}

	// shuffle the elements vector
	std::shuffle(elements.begin(), elements.end(), random_generator);

	// do it until there are no elements left
	while (elements.size())
	{
		int y = elements[elements.size() - 1].y;
		int x = elements[elements.size() - 1].x;
		int orientation = elements[elements.size() - 1].orientation;
		elements.pop_back();

		if (horizontal == orientation)
		{
			// if the two sets are different, join them and carve a passage
			if (sets[y][x + 2] != sets[y][x])
			{
				replace(sets[y][x + 2], sets[y][x]);
				for (int j = 0; j < 3; j++)
				{
					grid[y][x + j] = path;
				}
			}
		}
		else if (vertical == orientation)
		{
			// if the two sets are different, join them and carve a passage
			if (sets[y + 2][x] != sets[y][x])
			{
				replace(sets[y + 2][x], sets[y][x]);
				for (int j = 0; j < 3; j++)
				{
					grid[y + j][x] = path;
				}
			}
		}
		else
		{
			// do nothing
		}
	}
}

void MazeGeneratePNG::replace(int set_to_replace, int sample_set)
{
	for (int search_y = 1; search_y < ((int)sets.size() - 1); search_y += 2)
	{
		for (int search_x = 1; search_x < ((int)sets[0].size() - 1); search_x += 2)
		{
			if (sets[search_y][search_x] == set_to_replace)
			{
				sets[search_y][search_x] = sample_set;
			}
		}
	}
}

void MazeGeneratePNG::draw_png()
{
	// Color png

	setupColorDataForColor();

	std::vector<unsigned char> a;
	for (int y = 0; y < this->height; y++)
	{
		for (int x = 0; x < this->width; x++)
		{
			a.push_back(image[y][x][0]);
			a.push_back(image[y][x][1]);
			a.push_back(image[y][x][2]);
		}
	}


	if (!stbi_write_png("Bitmap/maze.png", this->width, this->height, 3, a.data(), 3 * width))
	{
		std::cout << "THIS SHIT FAILED maze.png not written" << std::endl;
	}
	else
	{
		std::cout << "maze.png written to Bitmap-folder" << std::endl;
	}




	// Black-White png

	setupColorData();

	std::vector<unsigned char> b;
	for (int y = 0; y < this->height; y++)
	{
		for (int x = 0; x < this->width; x++)
		{
			b.push_back(image[y][x][0]);
			b.push_back(image[y][x][1]);
			b.push_back(image[y][x][2]);
		}
	}


	if (!stbi_write_png("Bitmap/maze_d.png", this->width, this->height, 3, b.data(), 3 * width))
	{
		std::cout << "THIS SHIT FAILED maze_d.png not written" << std::endl;
	}
	else
	{
		std::cout << "maze_d.png written to Bitmap-folder" << std::endl;
	}
}

void MazeGeneratePNG::setupColorDataForColor()
{
	// set up image vectors to hold color data
	image.resize(this->height);
	for (int y = 0; y < this->height; y++)
	{
		image[y].resize(this->width);
		for (int x = 0; x < this->width; x++)
		{
			image[y][x].resize(3);
		}
	}

	// sets colors, white for walls, black for path
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (get_cell(y, x) == wall)
			{
				bool closeby[4]; // false for floor
				// Read closeby pixels
				// 
				//    _0_
				// 3 | . | 1
				//   |___|
				//	   2

				// Assume its surrounded by walls
				closeby[0] = wall;
				closeby[1] = wall;
				closeby[2] = wall;
				closeby[3] = wall;


				// If at the edge, keep the assumption else check if there is a floor or wall
				if (y == 0)
					closeby[2] = get_cell(y + 1, x);
				else if (y == height - 1)
					closeby[0] = get_cell(y - 1, x);
				else
				{
					closeby[0] = get_cell(y - 1, x);
					closeby[2] = get_cell(y + 1, x);
				}

				if (x == 0)
					closeby[1] = get_cell(y, x + 1);
				else if (x == width - 1)
					closeby[3] = get_cell(y, x - 1);
				else
				{
					closeby[1] = get_cell(y, x + 1);
					closeby[3] = get_cell(y, x - 1);
				}



				// Check which wall type it is based on closeby
				if (closeby[0] && closeby[1] && closeby[2] && closeby[3]) // 0. Empty
				{
					image[y][x][0] = 0;
					image[y][x][1] = 0;
					image[y][x][2] = 0;
				}
				else if (!closeby[0] && closeby[1] && closeby[2] && closeby[3]) // 1. Single wall closeby
				{
					image[y][x][0] = 100;
					image[y][x][1] = 0;
					image[y][x][2] = 255;
				}
				else if (closeby[0] && closeby[1] && !closeby[2] && closeby[3]) // 2.
				{
					image[y][x][0] = 100;
					image[y][x][1] = 255;
					image[y][x][2] = 0;
				}
				else if (closeby[0] && !closeby[1] && closeby[2] && closeby[3]) // 3.
				{
					image[y][x][0] = 100;
					image[y][x][1] = 255;
					image[y][x][2] = 255;
				}
				else if (closeby[0] && closeby[1] && closeby[2] && !closeby[3]) // 4.
				{
					image[y][x][0] = 100;
					image[y][x][1] = 0;
					image[y][x][2] = 0;
				}
				else if (!closeby[0] && !closeby[1] && closeby[2] && closeby[3]) // 5. Double wall closeby
				{
					image[y][x][0] = 200;
					image[y][x][1] = 0;
					image[y][x][2] = 255;
				}
				else if (!closeby[0] && closeby[1] && closeby[2] && !closeby[3]) // 6.
				{
					image[y][x][0] = 200;
					image[y][x][1] = 255;
					image[y][x][2] = 0;
				}
				else if (closeby[0] && closeby[1] && !closeby[2] && !closeby[3]) // 7.
				{
					image[y][x][0] = 200;
					image[y][x][1] = 0;
					image[y][x][2] = 0;
				}
				else if (closeby[0] && !closeby[1] && !closeby[2] && closeby[3]) // 8.
				{
					image[y][x][0] = 200;
					image[y][x][1] = 255;
					image[y][x][2] = 255;
				}
				else if (!closeby[0] && !closeby[1] && closeby[2] && !closeby[3]) // 9. Tripple wall closeby
				{
					image[y][x][0] = 255;
					image[y][x][1] = 0;
					image[y][x][2] = 255;
				}
				else if (!closeby[0] && !closeby[1] && !closeby[2] && closeby[3]) // 10.
				{
					image[y][x][0] = 255;
					image[y][x][1] = 255;
					image[y][x][2] = 0;
				}
				else if (closeby[0] && !closeby[1] && !closeby[2] && !closeby[3]) // 11.
				{
					image[y][x][0] = 255;
					image[y][x][1] = 0;
					image[y][x][2] = 0;
				}
				else if (!closeby[0] && closeby[1] && !closeby[2] && !closeby[3]) // 12.
				{
					image[y][x][0] = 255;
					image[y][x][1] = 255;
					image[y][x][2] = 255;
				}
				else if (closeby[0] && !closeby[1] && closeby[2] && !closeby[3]) // 13. Double wall closeby Ex
				{
					image[y][x][0] = 50;
					image[y][x][1] = 0;
					image[y][x][2] = 0;
				}
				else if (!closeby[0] && closeby[1] && !closeby[2] && closeby[3]) // 14.
				{
					image[y][x][0] = 50;
					image[y][x][1] = 255;
					image[y][x][2] = 255;
				}
			}
			else if (get_cell(y, x) == path)
			{
				for (int i = 0; i < 3; i++)
				{
					image[y][x][i] = 0;
				}
			}
		}
	}
}


void MazeGeneratePNG::setupColorData()
{
	// set up image vectors to hold color data
	image.resize(this->height);
	for (int y = 0; y < this->height; y++)
	{
		image[y].resize(this->width);
		for (int x = 0; x < this->width; x++)
		{
			image[y][x].resize(3);
		}
	}

	// sets colors, white for walls, black for path
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (get_cell(y, x) == wall)
			{
				for(int i = 0; i < 3; i++)
				{
					image[y][x][i] = 255;
				}
			}
			else if (get_cell(y, x) == path)
			{
				for (int i = 0; i < 3; i++)
				{
					image[y][x][i] = 0;
				}
			}
		}
	}
}
