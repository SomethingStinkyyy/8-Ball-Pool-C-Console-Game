//Programming For Engineer Project - Group 33
//Pool Game Project
//Members - Yasan, Malith, Dineth, Chathumal, Akila


// header files
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <limits>
#include <fstream>
#include <conio.h>
#include <cstdlib>


//libraries
using namespace std; 

#include "GameEngine.h" //including the external resource basic game engine

struct sBall //creating a structure to carry physics data about each pool ball on a 2D plane i.e. the table
{
	float px, py; //variables for position vector of the ball on the table
	float vx, vy; //variables for velocity vector of the ball
	float ax, ay; //variables for acceleration vector of the ball
	float ox, oy; //variables for the previous position vectors of the ball (oldx and oldy)
	float radius; //radius of the ball
	short color; //color of the ball
	int type; //type of the ball (red/stripe(1), yellow/full color(2), 8 black ball(3), or White cue ball(0))

	float fSimTimeRemaining; //variable to keep track of the simulated time remaining for each ball 
	                         //until the end of an epoch

	int id; //unique identifier for each ball
};

struct sLineSegment //creating a structure to carry data about the edges of the table (in the form of capsule line segments)
{
	float sx, sy; //coordinates for the Starting point of the line segment
	float ex, ey; //coordinates for the Ending point of the line segment
	float radius; //radius of the line segment (perpendicular distance from an imaginary central line inside the 
	              //table edge to to the edge of the capsule)
};

//Declaring variables and functions for the main menu

string PlayerName;
HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE); //setting up handler for font size in console
void fontsize(int, int);
HANDLE color = GetStdHandle(STD_OUTPUT_HANDLE); //setting up handler for font color in console


//initializing font size

void fontsize(int a, int b) {
	PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx = new CONSOLE_FONT_INFOEX();
	lpConsoleCurrentFontEx->cbSize = sizeof(CONSOLE_FONT_INFOEX);
	GetCurrentConsoleFontEx(out, 0, lpConsoleCurrentFontEx);
	lpConsoleCurrentFontEx->dwFontSize.X = a;
	lpConsoleCurrentFontEx->dwFontSize.Y = b;
	SetCurrentConsoleFontEx(out, 0, lpConsoleCurrentFontEx);
}

VOID WINAPI SetConsoleColors(WORD attribs) {
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFOEX cbi;
	cbi.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
	GetConsoleScreenBufferInfoEx(hOutput, &cbi);
	cbi.wAttributes = attribs;
	SetConsoleScreenBufferInfoEx(hOutput, &cbi);
}

int framesDelay;
int bossLife;
bool bossPlaced = false;
bool chasersPlaced = false;
char c;
bool exitConsole = false;
bool inGame;

int frameCount; // which represents its position in static array

void gotoxy(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
char getCursorChar()    // Function which returns character on console's cursor position 
{
	char c = '\0';
	CONSOLE_SCREEN_BUFFER_INFO con;
	HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hcon != INVALID_HANDLE_VALUE &&
		GetConsoleScreenBufferInfo(hcon, &con))
	{
		DWORD read = 0;
		if (!ReadConsoleOutputCharacterA(hcon, &c, 1,
			con.dwCursorPosition, &read) || read != 1
			)
			c = '\0';
	}
	return c;
}



class PoolGame : public GameEngine //defining a class for the physics of the pool balls, referring to the rudimentary game engine
{
public:
	PoolGame () //a public method within the class for physics of the pool balls
	{
		m_sAppName = L"8 Ball Pool Programming Project";
	}


private:
	vector<pair<float, float>> modelBall; //vector within the class that will be used in drawing the ball on the screen
	sBall *pSelectedBall = nullptr; //a private variable is added to the class, to act as a pointer to the cue ball

	vector<sLineSegment> vecLines; //vector to store the data of the table edges

	//pointer for front sprite to create text
	olcSprite* m_sprFont = nullptr; 

public:
	vector<sBall> vecBalls; //vector to store all the physics data of all the pool balls
	vector <sBall> vecPocketFakeBalls; //vector to store the fake balls at the pockets or pool table holes

	//utility function/method to easily add balls to the vecBalls vector
	void AddBall(float x, float y, short color, int type, float r = 5.0f) 
	{
		sBall b; //creating a instance of the sBall structure
		b.px = x; b.py = y; //accessing the variables of the structure to store data
		b.vx = 0; b.vy = 0;
		b.ax = 0; b.ay = 0;
		b.radius = r;
		b.color = color;
		b.type = type;

		b.id = vecBalls.size(); //id for each ball will be assigned based on the number of balls in vecBalls vector
		vecBalls.emplace_back(b); //putting the setup ball at the back of the vector
	}

	//function to draw text on the game screen
	void DrawBigText(int x, int y, string sText) {
		int i = 0;
		for (auto c : sText) 
		{
			//index into the sprite 
			int sx = ((c - 32) % 16) * 8;
			int sy = ((c - 32) / 16) * 8;
			DrawPartialSprite(x + i * 8, y, m_sprFont, sx, sy, 8, 8);
			i++;
		}
	}

public:
	bool OnUserCreate() //method/function for loading the resources in the game console
	{
		//init sprite font pointer - to read font sprite file
		m_sprFont = new olcSprite(L"pixelfont.spr");


		float fRadius = 5.0f; //default variable for the radius of the balls, allowing changes if required 

		//using the add ball function to add balls at specific locations of the game engine console
		//following are balls with specific locations
		AddBall(357, 165, FG_DARK_RED, 1, fRadius); //first ball

		AddBall(367, 160, FG_DARK_RED, 1, fRadius); //second row of two balls
		AddBall(367, 170, FG_DARK_YELLOW, 2, fRadius);

		AddBall(377, 165, FG_DARK_GREY, 3, fRadius); //third row of three balls. This is the 8 ball.
		AddBall(377, 175, FG_DARK_RED, 1, fRadius);
		AddBall(377, 155, FG_DARK_YELLOW, 2, fRadius);

		AddBall(387, 170, FG_DARK_RED, 1, fRadius); //fourth row of 4 balls
		AddBall(387, 180, FG_DARK_YELLOW, 2, fRadius);
		AddBall(387, 160, FG_DARK_YELLOW, 2, fRadius);
		AddBall(387, 150, FG_DARK_RED, 1, fRadius);

		AddBall(397, 165, FG_DARK_RED, 1, fRadius); //fifth row of 5 balls
		AddBall(397, 175, FG_DARK_YELLOW, 2, fRadius);
		AddBall(397, 155, FG_DARK_YELLOW, 2, fRadius);
		AddBall(397, 185, FG_DARK_YELLOW, 2, fRadius);
		AddBall(397, 145, FG_DARK_RED, 1, fRadius);
		


		AddBall(140, 165, FG_WHITE, 0, fRadius); //cue ball or white ball

		//NOTE: Cue Ball is type 0, Red is 1, Yellow is 2, and Black 8 ball is 3

		//adding the table edges to form the pool table

		float fLineRadius = 6.0f; //default variable for the radius of the table edges, allowing changes if required

		//horizontal edges
		vecLines.push_back({ 50.0f, 54.0f, 236.0f, 54.0f, fLineRadius });
		vecLines.push_back({ 264.0f, 54.0f, 450.0f, 54.0f, fLineRadius });
		vecLines.push_back({ 50.0f, 276.0f, 236.0f, 276.0f, fLineRadius });
		vecLines.push_back({ 264.0f, 276.0f, 450.0f, 276.0f, fLineRadius });

		//vertical edges
		vecLines.push_back({ 30.0f, 74.0f, 30.0f, 256.0f, fLineRadius });
		vecLines.push_back({ 470.0f, 74.0f, 470.0f, 256.0f, fLineRadius });
		
		return true;
	}

	int player = 0; //variable to represent which players turn it is
	bool firstpot = 0; //variable to decide whether a pot is the first one or not
	bool P1_Red=0; //variable to determine whether red is allocated for player 1 or not

	int Score = 0; //variables for calculating the score or balls remaining to be potted
	int P1_Score = 0;
	int P2_Score = 0;

	//no. of red and yellow balls at the start of the game
	int r_balls = 7;
	int y_balls = 7;

	//variable for game to end
	bool gameend = 0;

	bool OnUserUpdate(float fElapsedTime) //method/function for refreshing the console once every frame (avoids jittering and slow fps for the pool game)
	{
		//auto variable to determine whether two balls are touching or not using their radii and position vectors of centers
		auto BallsHaveCollided = [](float x1, float y1, float r1, float x2, float y2, float r2)
		{
			return ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) <= (r1 + r2) * (r1 + r2);
			/*checking if the difference between the position vectors of the two balls squared, are less than or
			equal to the sum of squares of their radii. This is pythagorus theorem.
			This function will RETURN a boolean whether this condition is true or not.
			If true, they will have collided.*/
		};

		//auto variable to determine whether a ball has been selected and contains a point (IsPointInBall)
		auto IsBallSelected = [](float x1, float y1, float r1, int px, int py) //contains variables for x and y
																				   //coordinates of pointer
		{
			return ((x1 - px) * (x1 - px) + (y1 - py) * (y1 - py)) < (r1 * r1); //returns bool result
		};

		if (m_mouse[0].bPressed || m_mouse[1].bPressed) //0 refers to left click and 1 to right click, so only enabled for right click
		{
			pSelectedBall = nullptr; //set to null pointer when clicked elsewhere 

			for (auto& ball : vecBalls) //for loop using IsBallSelected auto variable to check what ball has been selected
			{
				if (IsBallSelected(ball.px, ball.py, ball.radius, m_mousePosX, m_mousePosY))
				{
					pSelectedBall = &ball; //if condition is true, selected ball is set to the address the ball that 
										   //gave a true condition
					break;
				}
			}
		}

		if (m_mouse[0].bHeld) //for case when the ball is being dragged and dropped
		{
			if (pSelectedBall != nullptr  && pSelectedBall->type == 0)
			{
				pSelectedBall->px = m_mousePosX;
				pSelectedBall->py = m_mousePosY;
			}
		}

		if (m_mouse[0].bReleased) //considering case when the left mouse button is released
		{
			pSelectedBall = nullptr;
		}

		if (m_mouse[1].bReleased) //considering case when the right mouse button is released
															  //and limiting this capability for the white ball
		{
			if (pSelectedBall != nullptr && pSelectedBall->type == 0)
			{
				//Applying the velocity to the selected ball
				pSelectedBall->vx = 5.0f * ((pSelectedBall->px) - (float)m_mousePosX);
				pSelectedBall->vy = 5.0f * ((pSelectedBall->py) - (float)m_mousePosY);
				//Here the difference between coordinates of ball center and coordinates of mouse pointer are taken
				//and multiplied by 5 to get a velocity vector. This 5 is a guesstimate. 

				player = player + 1; //operation to change the player allocation at the end of each turn
			}
			pSelectedBall = nullptr; //the pointer is emptied to prevent any further calculations and console drawings

		}

		//recording all the pairs of balls that have collided by using a vector
		vector<pair<sBall*, sBall*>> vecCollidingPairs;

		//creating a vector to store the fake balls, that refer to table edge collisions
		vector <sBall*> vecFakeBalls;


		//Making the Simulation more realistic and accurate, even in the case of low frame rates

		int nSimulationUpdates = 4; //the physics simulation will be run this many times for every frame update

		float fSimElapsedTime = fElapsedTime / (float)nSimulationUpdates;
		//instead of using Elapsed Time from the engine itself, it is now subdivided for better accuracy
		//This will be the new epoch time. 

		int nMaxSimulationSteps = 15; //variable to determine how many times the static and dynamic collisions will be 
									  //resolved per epoch

		//MAIN SIMULATION LOOP (PHYSICS CODE)
		for (int i = 0; i < nSimulationUpdates; i++)
		{

			//Set the time remaining of all balls to maximum for this particular epoch
			for (auto& ball : vecBalls)
				ball.fSimTimeRemaining = fSimElapsedTime;

			for (int j = 0; j < nMaxSimulationSteps; j++)
			{
				//Updating the Positions of the Balls, to give physics to the balls, by using an auto for loop.
				//Here the effect on the balls due to acceleration and velocity is also considered, by using the elapsed time
				for (auto& ball : vecBalls)
				{
					if (ball.fSimTimeRemaining > 0.0f)
					{
						ball.ox = ball.px; //recording the original positions of the ball before the collision
						ball.oy = ball.py;

						//Add Drag to emulate rolling Friction
						/*drag or friction is added, by setting the acceleration component of the ball to be a proportion of
						the velocity in the opposite direction*/
						ball.ax = -ball.vx * 0.8f;
						ball.ay = -ball.vy * 0.8f;

						//Updating ball physics
						ball.vx += ball.ax * ball.fSimTimeRemaining; //thus, resultant velocity vector in x direction
						ball.vy += ball.ay * ball.fSimTimeRemaining; //resultant velocity vector in y direction
						ball.px += ball.vx * ball.fSimTimeRemaining; //resultant position vector in x direction
						ball.py += ball.vy * ball.fSimTimeRemaining; //resultant position vector in y direction

						// Wrap the cue ball around the screen, for when it is shot out of the table
						{
							if (ball.px < 0) ball.px += (float)ScreenWidth();
							if (ball.px >= ScreenWidth()) ball.px -= (float)ScreenWidth();
							if (ball.py < 0) ball.py += (float)ScreenHeight();
							if (ball.py >= ScreenHeight()) ball.py -= (float)ScreenHeight();
						}

						//Clamping the velocity when it nears 0
						//condition for the ball to stop after moving for some time
						if ((ball.vx * ball.vx + ball.vy * ball.vy) < 0.01f)
							/*i.e. if the square of the overall scalar speed of the velocity vector is less than 0.01, it is
							  clamped to 0*/
						{
							ball.vx = 0;
							ball.vy = 0;
						}
					}
				}


				//Implementing collision detection (Static Collision i.e. Overlap)
				for (auto& ball : vecBalls) //range based for loop to represent the balls
				{

					//Static Collisions against Table Edges
					for (auto& edge : vecLines)
					{
						//NOTE:The line along the center or middle of the table edge is taken as the edge X axis, and
						// the axis perpendicular to that as the Y axis.

						//Determining the components of the table edge vector (Vector A)
						float fLineX1 = edge.ex - edge.sx;
						float fLineY1 = edge.ey - edge.sy;

						//Determining the vector for the line segment from the center of the colliding ball to the starting point
						//of the table edge (Vector B)
						float fLineX2 = ball.px - edge.sx;
						float fLineY2 = ball.py - edge.sy;

						//Determining the length of the table edge segment
						float fEdgeLength = fLineX1 * fLineX1 + fLineY1 * fLineY1;

						/*Determining the length from ball center to normal point on the table edge.
						/ This is found by taking dot product of two vectors A and B, and dividing by the edge length
						  But we also have to consider whether the ball is more closer to the starting point or the
						  ending point of the table edge capsule segment*/
						float t = max(0, min(fEdgeLength, (fLineX1 * fLineX2 + fLineY1 * fLineY2))) / fEdgeLength;

						//Finding coordinates of the Closest Point on the table edge to the ball.
						//We will come along the starting point to t multiplied by the gradient for the edge X axis.
						float fClosestPointX = edge.sx + t * fLineX1;
						float fClosestPointY = edge.sy + t * fLineY1;

						//Distance between the above Closest Point to the Center of the Ball
						float fDistance = sqrtf((ball.px - fClosestPointX) * (ball.px - fClosestPointX) + (ball.py - fClosestPointY) * (ball.py - fClosestPointY));

						//Determining if a Collision has occured
						if (fDistance <= (ball.radius + edge.radius))
						{
							//if true, a Static Collision has occured

							//Collision parameters are calculated by considering a fake ball inside the table edge capsule
							sBall* fakeball = new sBall();
							fakeball->radius = edge.radius; //set its radius as capsule edge radius

							//setting center coordinates of the fake ball as those of the closest point
							fakeball->px = fClosestPointX;
							fakeball->py = fClosestPointY;

							//setting the velocity of the fake ball to be exactly opposite of that of the collidng ball
							//this will act like a ball hitting a solid surface
							fakeball->vx = -ball.vx;
							fakeball->vy = -ball.vy;

							//storing the fake ball i.e. point of table edge collision in the relevant vector
							vecFakeBalls.push_back(fakeball);

							//storing the collding ball and fake ball in the colliding pairs to be dynamically resolved
							//later on
							vecCollidingPairs.push_back({ &ball, fakeball });

							//resolving the static collision in the same way as for ball collisions (in depth explanation below)
							//but without taking half as this is a table edge which is fixed and stationary
							float fOverlap = 1.0f * (fDistance - ball.radius - fakeball->radius);

							//So Displace the Colliding Ball away from the point of Collision as so,
							ball.px -= fOverlap * (ball.px - fakeball->px) / fDistance;
							ball.py -= fOverlap * (ball.py - fakeball->py) / fDistance;
							//i.e. the ball is Displaced by a magnitude equal to fOverlap, along its velocity vector
						}
					}


					//Static Collision among Balls
					for (auto& target : vecBalls) //for loop to represent the target ball
					{
						if (ball.id != target.id) //using ball ids to prevent self-collision of balls with themselves
						{
							if (BallsHaveCollided(ball.px, ball.py, ball.radius, target.px, target.py, target.radius))
								//sending variables of the two balls in question to the auto variable to return a bool answer
							{
								/*If a Collision has occured, the addresses of those pairs of balls are added to the
								vector vecCollidingPairs*/
								vecCollidingPairs.push_back({ &ball, &target });

								/*Finding the distance between ball centers by taking the square root of the sum of x and y
								of the cue and target balls*/
								float fDistance = sqrtf((ball.px - target.px) * (ball.px - target.px) + (ball.py - target.py) * (ball.py - target.py));

								/*Finding the length of the overlap by substracting two radii from the distance between centers.
								Half of this value is taken to be the distance by which each ball is displaced. */
								float fOverlap = 0.5f * (fDistance - ball.radius - target.radius);

								//Considering only the static collision
								//Displacing the current ball by directly altering its current position
								ball.px -= fOverlap * (ball.px - target.px) / fDistance;
								ball.py -= fOverlap * (ball.py - target.py) / fDistance;
								//- sign is added here to make the cue ball move away from the target ball
								// Ball will move fOverlap distance, in the direction of the vector created between the centers 
								// of the two balls. 
								//So fOverlap, is multiplied by the difference between the respective coordinates of the position 
								// vectors of the centers. 
								//This is then normalized(normal vector) by dividing by the distance between the centers.

								//Displacing the target ball in the exact same way
								target.px += fOverlap * (ball.px - target.px) / fDistance;
								target.py += fOverlap * (ball.py - target.py) / fDistance;
								//+ sign is used here to displace the target ball away from the direction of the cue ball
							}
						}
					}

					//Our basic code takes an approximation, where the balls intersect over each other
					// This correction will make this more accurate and prevent possible errors in low fps instances

					//Time displacement (how much the epoch time has to be reduced by, per static collision)
					float fIntendedSpeed = sqrtf(ball.vx * ball.vx + ball.vy * ball.vy); //magnitude of velocity vector
					float fIntendedDistance = fIntendedSpeed * ball.fSimTimeRemaining; //s=ut

					//In case of collision, it would not travel the entire intended distance, so,
					float fActualDistance = sqrtf((ball.px - ball.ox) * (ball.px - ball.ox) + (ball.py - ball.oy) * (ball.py - ball.oy));
					//which is the magnitude of difference between initial and final position vectors

					//so actual time passed due to the collision
					float fActualTime = fActualDistance / fIntendedSpeed;

					//updating the time remaining for the end of the epoch
					ball.fSimTimeRemaining = ball.fSimTimeRemaining - fActualTime;
				}

				//Now working out the dynamic collision
				for (auto c : vecCollidingPairs) //range based for loop to auto assign 
				{
					sBall* b1 = c.first; //structure variable to represent first ball colliding
					sBall* b2 = c.second; //structure variable to represent second ball colliding

					//Calculating the distance between two balls
					float fDistance = sqrtf((b1->px - b2->px) * (b1->px - b2->px) + (b1->py - b2->py) * (b1->py - b2->py));

					//Determining the normal vector at the point of collision 
					float nx = (b2->px - b1->px) / fDistance;
					float ny = (b2->py - b1->py) / fDistance;
					//Normalized by dividing by distance

					//Determining the normalized tangent vector at the point of collision 
					float tx = -ny;
					float ty = nx;
					//it is the same as the normal vector, except the coordinates are inverted and x is taken as negative

					//Normal and Tangent vectors refer to the equal and opposite vectors acting on the two colliding balls

					/*The tangential response of the collision, can be considered to be the Dot Product of the velocity vector
					  of the ball and the tangential vector. NOTE: dot product gives a scalar value*/
					float dpTan1 = b1->vx * tx + b1->vy * ty;
					float dpTan2 = b2->vx * tx + b2->vy * ty;

					//Similarly, Normal response of the collision is the Dot Product of the velocity vector and normal vector
					float dpNorm1 = b1->vx * nx + b1->vy * ny;
					float dpNorm2 = b2->vx * nx + b2->vy * ny;

					/*Assume Elastic Collisions, and equal mass for all the balls.
					  Then, when momentum and energy conservation is applied, momentums of the balls m1, m2 are given by,*/
					float m1 = dpNorm2;
					float m2 = dpNorm1;

					/*Updating the Ball Velocities.
					  The Scalar value of velocity from the tangential response, which gives magnitude, is multipled by the
					  normalized tangential vector, which gives direction. Similarly, the normal response velocity is multipled
					  by the normal vector. Then, their sum is taken.*/
					b1->vx = tx * dpTan1 + nx * m1;
					b1->vy = ty * dpTan1 + ny * m1;
					b2->vx = tx * dpTan2 + nx * m2;
					b2->vy = ty * dpTan2 + ny * m2;
				}

				//after resolving all of the pairs, it is no longer needed to allocate memory for the fake balls considered
				//for the table edges
				//So Remove the Fake Balls
				for (auto& b : vecFakeBalls) delete b;
				vecFakeBalls.clear();

				//Remove All Colliding Pairs as well (to save memory)
				vecCollidingPairs.clear();
			}



		}



		//CODE FOR DRAWING ON CONSOLE SCREEN

		if (gameend != 1) //condition for the game to end
	{
		//First of all, clearing the screen by using the fill function of the game engine
		Fill(0, 0, ScreenWidth(), ScreenHeight(), ' ');

		//Drawing the edges of the table on the screen
		for (auto& line : vecLines)
		{
			FillCircle(line.sx, line.sy, line.radius, PIXEL_HALF, FG_DARK_YELLOW);
			FillCircle(line.ex, line.ey, line.radius, PIXEL_HALF, FG_DARK_YELLOW);
			//(This will draw two circles at the edges of each table edge)

			/*Two draw the actual edges we will be using a unit vector perpendicular to the line joining
			  above two circles, and then multiply it by the radius of one circle.*/

			  //x and y components of said normal vector
			float nx = -(line.ey - line.sy);
			float ny = (line.ex - line.sx);

			//length of the vector
			float d = sqrt(nx * nx + ny * ny);

			//unit vector of the normal
			nx /= d;
			ny /= d;

			//the normal unit vector is made use of two draw lines above and below the line joining the two circles
			DrawLine((line.sx + nx * line.radius), (line.sy + ny * line.radius), (line.ex + nx * line.radius), (line.ey + ny * line.radius), PIXEL_SOLID, FG_DARK_YELLOW);
			DrawLine((line.sx - nx * line.radius), (line.sy - ny * line.radius), (line.ex - nx * line.radius), (line.ey - ny * line.radius), PIXEL_SOLID, FG_DARK_YELLOW);
			//DrawLine engine function will draw lines on the console joining the edges of the two circles, and thus creating a table edge capsule segment.
		}

		//Drawing the pot holes or pockets of the pool table
		DrawCircle(35, 59, 8, PIXEL_HALF, BG_WHITE);
		DrawCircle(465, 59, 8, PIXEL_HALF, BG_WHITE);

		DrawCircle(35, 271, 8, PIXEL_HALF, BG_WHITE);
		DrawCircle(465, 271, 8, PIXEL_HALF, BG_WHITE);

		DrawCircle(250, 55, 8, PIXEL_HALF, BG_WHITE);
		DrawCircle(250, 275, 8, PIXEL_HALF, BG_WHITE);

		//range based for loop to Draw the Balls, using the automatic variable ball. 
		for (auto& ball : vecBalls) //NOTE: & was added to fix warning
		{
			FillCircle(ball.px, ball.py, ball.radius, PIXEL_SOLID, ball.color);
		}
		/*FillCircle engine function will draw filled circles on the console based on the data in modelBall vector.
		  Then the current position of the ball is used to offset the ball model.
		  Finally, the ball is scaled by its radius and drawn in a specific colour*/

		if (pSelectedBall != nullptr && pSelectedBall->type == 0)
		{
			//Drawing the pool cue on the console screen
			DrawLine(pSelectedBall->px, pSelectedBall->py, m_mousePosX, m_mousePosY, PIXEL_SOLID, FG_BLUE);
		}

		//Pointing out which players turn it is

		if (player % 2 == 0)
		{
			DrawBigText(20, 30, "Shoot!");
			DrawBigText(400, 30, "     ");
		}

		else if (player % 2 == 1)
		{
			DrawBigText(400, 30, "Shoot!");
			DrawBigText(20, 30, "     ");
		}

		//Drawing Text on the Screen i.e. Game UI and Game Functionality

		DrawBigText(20, 10, "PLAYER 1");
		DrawBigText(400, 10, "PLAYER 2");


		DrawBigText(20, 20, "Score: ");
		DrawBigText(400, 20, "Score: ");
	}

		//IMPLEMENTING THE POTTING OF BALLS i.e. making the balls fall into the holes (and destroying the balls)

		for (auto& b : vecBalls)
		{
			if (((b.px < 40 || b.px > 460) && (b.py < 64 || b.py > 271)) || (b.py < 52 || b.py > 278))
			{
				switch (b.type)
				{
				case 0:
				{
					//cue ball has been potted

				    //then chance is given to the other player
					break;
					
				}

				case 1:
				{
					//red ball has been potted
					r_balls -= 1;

					b.px = 486; //teleport balls outside the table
					b.py = 100;
					b.vx = 0; //stop balls after they go outside
					b.vy = 0;

					
					if (player % 2 == 1) //the moment player 1 finishes shooting player variable gets + 1
							                 //so the remainder will be 1, after player 1 shoots
						{
							if (firstpot == 0)
							{
								//first pot was a red ball by player 1
								P1_Red = 1; //so gets red
							}

							if (P1_Red == 1) //if red is player 1 color
							{
								player = player + 1; //allows player 1 to keep shooting even after the first pot
							}
						}

					else if (player % 2 == 0) //this will happen when player 2 makes the shot
						{
						if (firstpot == 0)
						{
							//first pot was red ball by player 2
							P1_Red = 0; //so player 1 will not get red
						}

						if (P1_Red == 0) //if red is players 2 color
						{
							player = player + 1; //allows player 2 to keep shooting
						}
						}

					firstpot = 1; //the first pot has already been done
					

					break;
				}

				case 2:
				{
					//yellow ball has been potted
					y_balls -= 1;

					b.px = 18; //teleport balls outside the table
					b.py = 100;
					b.vx = 0; //stop balls after they go outside
					b.vy = 0;

					
					if (player % 2 == 1) //player 1 made the shot
						{
							if (firstpot == 0) 
							{
								//first pot was yellow ball by player 1
								P1_Red = 0; //so player one will not get red
							}

							if (P1_Red == 0) //if yellow is player 1 color
							{
								player = player + 1; //allows player 1 to keep playing
							}
						}

					else if (player % 2 == 0)
						{
							if (firstpot == 0)
							{
								//first pot was yellow ball by player 2
								P1_Red = 1; //so player one will get red
							}

							if (P1_Red == 1) //if yellow is player 2 color
							{
								player = player + 1; //allows player 2 to keep playing
							}
						}
					firstpot = 1; //first pot has already been done

					break;
				}

				case 3:
				{
					//8 ball has been potted
					b.px = 18; //teleport balls outside the table
					b.py = 100;
					b.vx = 0; //stop balls after they go outside
					b.vy = 0;

					int p = player % 2; //if p = 1 then it is player 1, if p = 0 then it is player 2

					switch (p) //coding the win conditions
					{
					case 0:
					{
							if ((P1_Red == 0 && r_balls == 0) || (P1_Red == 1 && y_balls == 0))
							{
								gameend = 1;
								Fill(0, 0, ScreenWidth(), ScreenHeight(), ' ', 0x0000);
								DrawBigText(ScreenWidth() / 2, ScreenHeight() / 2, "PLAYER 2 WINS!");
								DrawBigText(ScreenWidth() / 2, (ScreenHeight() / 2) + 20, "Exit the Game");
							}

							else
							{
								gameend = 1;
								Fill(0, 0, ScreenWidth(), ScreenHeight(), ' ', 0x0000);
									DrawBigText(ScreenWidth() / 2, ScreenHeight() / 2, "PLAYER 1 WINS!");
								DrawBigText(ScreenWidth() / 2, (ScreenHeight() / 2) + 20, "Exit the Game");
							}
				
						break;
					}

					case 1:
					{
						if ((P1_Red == 1 && r_balls == 0) || (P1_Red == 0 && y_balls == 0))
						{
							gameend = 1; 
							Fill(0, 0, ScreenWidth(), ScreenHeight(), ' ');
							DrawBigText(ScreenWidth()/2, ScreenHeight()/2, "PLAYER 1 WINS!");
							DrawBigText(ScreenWidth() / 2, (ScreenHeight() / 2) + 20, "Exit the Game");
						}

						else
						{
							gameend = 1;
							Fill(0, 0, ScreenWidth(), ScreenHeight(), ' ');
							DrawBigText(ScreenWidth() / 2, ScreenHeight() / 2, "PLAYER 2 WINS!");
							DrawBigText(ScreenWidth() / 2, (ScreenHeight() / 2) + 20, "Exit the Game");
						}

						break;
					}
					break;
					}
					break;
				}
				break;
				}
			}
		}
				

		//Determining what color is given for the players
		
		if (firstpot != 0)
		{
			if (P1_Red == 1)
			{
				DrawBigText(90, 10, "(R)");
				DrawBigText(470, 10, "(Y)");
			}

			else if (P1_Red == 0)
			{
				DrawBigText(90, 10, "(Y)");
				DrawBigText(470, 10, "(R)");
			}
		}

		//Determining the score for each player
		if (P1_Red == 1)
		{
			P1_Score = 7 - r_balls;
			P2_Score = 7 - y_balls;
		}

		else if (P1_Red == 0)
		{
			P1_Score = 7 - y_balls;
			P2_Score = 7 - r_balls;
		}

		switch (P1_Score) //score for player 1
			{
			case 0:
			{
				DrawBigText(70, 20, "0");
				break;
			}

			case 1:
			{
				DrawBigText(70, 20, "1");
				break;
			}

			case 2:
			{
				DrawBigText(70, 20, "2");
				break;
			}

			case 3:
			{
				DrawBigText(70, 20, "3");
				break;
			}

			case 4:
			{
				DrawBigText(70, 20, "4");
				break;
			}

			case 5:
			{
				DrawBigText(70, 20, "5");
				break;
			}

			case 6:
			{
				DrawBigText(70, 20, "6");
				break;
			}

			case 7:
			{
				DrawBigText(70, 20, "8 Ball!");
				break;
			}

			default:
			{
				break;
			}
			break;
			}

			switch (P2_Score) //score for player 2
			{
			case 0:
			{
				DrawBigText(450, 20, "0");
				break;
			}

			case 1:
			{
				DrawBigText(450, 20, "1");
				break;
			}

			case 2:
			{
				DrawBigText(450, 20, "2");
				break;
			}

			case 3:
			{
				DrawBigText(450, 20, "3");
				break;
			}

			case 4:
			{
				DrawBigText(450, 20, "4");
				break;
			}

			case 5:
			{
				DrawBigText(450, 20, "5");
				break;
			}

			case 6:
			{
				DrawBigText(450, 20, "6");
				break;
			}

			case 7:
			{
				DrawBigText(450, 20, "8 Ball!");
				break;
			}

			default:
			{
				break;
			}
			break;
			}

		return true;
	}

};

#define MAX_X 1000 /*defining the values for the screen size of the console. NOTE: Some smaller screens may cause 
                     problems in the console screen size and adjusting these values correctly will fix the problem*/
#define MAX_Y 600
#define PIX_X 2
#define PIX_Y 2

int main() //main function for the pool game
{
	PoolGame PoolBalls; //Creating an object/subclass of the PoolGame class

	//Code for the Main Menu with Keyboard Input
		framesDelay = 10;
		string Menu[5] = { "Play", "How to play", "Rules of Pool", "Credits", "Exit" };
		int pointer = 0;

		while (true)
		{
			system("cls");

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			cout << "MAIN MENU";
			cout << "\n(use the arrow keys)\n\n";

			for (int i = 0; i < 5; ++i)
			{
				if (i == pointer)
				{
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
					cout << Menu[i] << endl;
				}
				else
				{
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
					cout << Menu[i] << endl;
				}
			}

			while (true)
			{
				if (GetAsyncKeyState(VK_UP) != 0)
				{
					pointer -= 1;
					if (pointer == -1)
					{
						pointer = 2;
					}
					break;
				}
				else if (GetAsyncKeyState(VK_DOWN) != 0)
				{
					pointer += 1;
					if (pointer == 5)
					{
						pointer = 0;
					}
					break;
				}
				else if (GetAsyncKeyState(VK_RETURN) != 0)
				{
					switch (pointer)
					{
					case 0:
					{
						if (PoolBalls.ConstructConsole(MAX_X / PIX_X, MAX_Y / PIX_Y, PIX_X, PIX_Y))
							PoolBalls.Start();

						else
							wcout << L"Could not construct console" << endl;
					} break;

					case 1:
					{   system("CLS");
					Sleep(50);
					cin.clear();
					cin.ignore();
					cout << "\t\tINSTRUCTIONS\n" << endl;
					cout << "1. Play multiplayer with two players taking turns, or play yourself\n" << endl;
					cout << "2. Hold and drag on the Cue Ball (White) with the Right Mouse Button to strike."
					      "\n   The Angle of Striking can be adjusted by holding and moving the cursor."
					      "\n   Release to Shoot.\n" << endl;
					cout << "3. The Left Mouse Button can be used to hold and drag the cue ball."
						  "\n   Note: This option should only be used when a player pots the cue ball"
						  "\n   or else it will be against the rules. Read RULES OF POOL for more info.\n" << endl;
					cout << "4. Pot all the balls of the allocated color and finally pot the 8 ball (Grey)"
						  "\n   to win.\n";
					Sleep(15000);
					} break;

					case 2:
					{
						system("CLS");

						cout << "\t\tRULES OF POOL\n" << endl;
						cout << "1. Insted of balls of multiple colors, this game is played with only two colors,"
							  "\n   namely, Yellow and Orange. (as in English Pool)\n" << endl;
						cout << "2. The Colors will be allocated to each player based on the color of the ball that is"
							  "\n   potted first."
							  "\n   eg:- If Player 1 pots a red ball first, they will get Red (R) and player 2"
							  "\n   will get Yellow(Y).\n" << endl;
						cout << "3. Players should aim to pot all the balls of their respective color."
							  "\n   For every ball potted, the player will get an additional chance to shoot."
							  "\n   If a player pots a ball of the wrong color, the other player will get a point."
							  "\n   Then no additonal chances will be given.\n" << endl;
						cout << "4. If a player pots the Cue (white) ball, then the other player will be given the chance"
							  "\n   to place the cue ball at any location and shoot. This is the only instance the"
							  "\n   moving of the cue ball with the left mouse button is allowed.\n" << endl;
						cout << "5. To win, a player should pot all the balls of their respective color"
							  "\n   and pot the 8 (Grey) Ball.\n" << endl;
						cout << "6. If a player pots the black ball before this, it will be an automatic defeat.\n"
							  "\n   So the other player will win.\n" << endl;

						Sleep(30000);
						break;
					}

					case 3:
					{
						system("CLS");

						cout << "\t\tCREDITS\n" << endl;
						cout << "\n\tYASAN\n"
							"\n\tMALITH\n"
							"\n\tCHATHUMAL\n"
							"\n\tDINETH\n"
							"\n\tAKILA\n";
							    
						Sleep(2000);
					}
					    break;

					case 4:
					{
						return 0;
					} break;
					}
					break;
				}
			}

			Sleep(150);
		}

		return 0;
}
