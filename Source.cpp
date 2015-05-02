#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <random>
#include <algorithm>
#include <vector>
#include <tuple>

#define BDIM 10
#define NOP 2097152
#define K 20971

void progressBarDisplay(float progress) {
	int barWidth = 70;

	std::cout << "[";
	int pos = barWidth * progress;
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) std::cout << "=";
		else if (i == pos) std::cout << ">";
		else std::cout << " ";
	}
	std::cout << "] " << int(progress * 100.0) << " %\r";
	std::cout.flush();
}


void createParticleGrid()
{
  FILE *fIn;
  FILE *fOut;
  double *inData;
  double *outData;
  inData = new double[NOP*3];
  

  fIn = fopen("particlePostion.raw","rb");


  fOut = fopen("CparticleGrid64.raw","wb");


  if(!fIn)
  {
      fprintf(stderr, "Error opening file\n");
      exit(13);
  }

  fread(inData, sizeof(double), NOP*4, fIn);
  fclose(fIn);

	//We determine the optimal grid resolution 
	double xMin,xMax,yMin,yMax,zMin,zMax;
	xMin = yMin = zMin = 999999.9;
	xMax = yMax = zMax = -999999.9;

	for(int i = 0; i < NOP; i++) {
		if(inData[3*i+0]>xMax) {
			xMax = inData[3*i+0];
		}
		if(inData[3*i+0]<xMin) {
			xMin = inData[3*i+0];
		}
		if(inData[3*i+1]>yMax) {
			yMax = inData[3*i+1];
		}
		if(inData[3*i+1]<yMin) {
			yMin = inData[3*i+1];
		}
		if(inData[3*i+2]>zMax) {
			zMax = inData[3*i+2];
		}
		if(inData[3*i+2]<zMin) {
			zMin = inData[3*i+2];
		}
	}

	//Now we determine the cpud 
	double xLen = xMax - xMin;
	double yLen = yMax - yMin;
	double zLen = zMax - zMin;

	double delta = std::max(std::max(xLen, yLen), zLen);
	double majorAxisLenght = 1.001;

	double cpud = (cbrt(NOP)*majorAxisLenght) / delta;

	//int gridResX = xLen * cpud;
	//int gridResY = yLen * cpud;
	//int gridResZ = zLen * cpud;

	int gridResX = 64;
	int gridResY = 64;
	int gridResZ = 64;

	outData = new double[gridResX * gridResY * gridResZ];

	int perGridLenX = xLen / gridResX;
	int perGridLenY = yLen / gridResY;
	int perGridLenZ = zLen / gridResZ;

	std::cout << "[INFO] Determined Grid Resolution as " << gridResX << " x " << gridResY << " x " << gridResZ << "\n";

	//Grid insertion point 
	std::cout << "[INFO] Trying to fit the particles into their homes in the grid.\n";
	std::vector< std::vector< std::vector< std::vector< int > > > > pointsInGrid;
	pointsInGrid.resize(gridResX);
	for (int j = 0; j < gridResX; j++){
		pointsInGrid[j].resize(gridResY);
		for (int k = 0; k < gridResY; k++){
			pointsInGrid[j][k].resize(gridResZ);
		}
	}

	for (int i = 0; i < NOP; i++){
		double xC = inData[3 * i + 0];
		double yC = inData[3 * i + 1];
		double zC = inData[3 * i + 2];

		int xLoc = ((xC - xMin) * (gridResX-1)) / xLen;
		int yLoc = ((yC - yMin) * (gridResY-1)) / yLen;
		int zLoc = ((zC - zMin) * (gridResZ-1)) / zLen;

		pointsInGrid[xLoc][yLoc][zLoc].push_back(i);

	}

	//Now we will do the reverse wighed interpolation 
	std::cout << "[INFO] Started creating the actual grid.\n";
	double progressCounter = 0.0;
	int totalProgress = gridResX*gridResY*gridResZ;
	progressBarDisplay(0.0);
	for (int i = 0; i < gridResX; i++){
		for (int j = 0; j < gridResY; j++){
			for (int k = 0; k < gridResZ; k++){

				//There can be max of 8 cells and min of 6 cells 
				std::vector<int> allPoints;
	
				std::vector<int> grid1 = pointsInGrid[i][j][k];
				allPoints.resize(grid1.size());
				std::copy(grid1.begin(), grid1.end(), allPoints.begin());

				if (j - 1 >= 0) {
					std::vector<int> grid2 = pointsInGrid[i][j - 1][k];
					int curSize = allPoints.size();
					int newSize = curSize + grid2.size();
					allPoints.resize(newSize);
					std::copy(grid2.begin(), grid2.end(), allPoints.begin() + curSize);
				}

				if (k - 1 >= 0){
					std::vector<int> grid3 = pointsInGrid[i][j][k - 1];
					int curSize = allPoints.size();
					int newSize = curSize + grid3.size();
					allPoints.resize(newSize);
					std::copy(grid3.begin(), grid3.end(), allPoints.begin() + curSize);
				}

				if ((j - 1 >= 0) && (k - 1 >= 0)) {
					std::vector<int> grid4 = pointsInGrid[i][j - 1][k - 1];
					int curSize = allPoints.size();
					int newSize = curSize + grid4.size();
					allPoints.resize(newSize);
					std::copy(grid4.begin(), grid4.end(), allPoints.begin() + curSize);
				}

				if (i - 1 >= 0) {
					std::vector<int> grid5 = pointsInGrid[i - 1][j][k];
					int curSize = allPoints.size();
					int newSize = curSize + grid5.size();
					allPoints.resize(newSize);
					std::copy(grid5.begin(), grid5.end(), allPoints.begin() + curSize);
				}

				if ((i - 1 >= 0) && (j - 1 >= 0)) {
					std::vector<int> grid6 = pointsInGrid[i - 1][j - 1][k];
					int curSize = allPoints.size();
					int newSize = curSize + grid6.size();
					allPoints.resize(newSize);
					std::copy(grid6.begin(), grid6.end(), allPoints.begin() + curSize);
				}

				if ((i - 1 >= 0) && (k - 1 >= 0)){
					std::vector<int> grid7 = pointsInGrid[i - 1][j][k - 1];
					int curSize = allPoints.size();
					int newSize = curSize + grid7.size();
					allPoints.resize(newSize);
					std::copy(grid7.begin(), grid7.end(), allPoints.begin() + curSize);
				}

				if ((i - 1 >= 0) && (j - 1 >= 0) && (k - 1 >= 0)) {
					std::vector<int> grid8 = pointsInGrid[i - 1][j - 1][k - 1];
					int curSize = allPoints.size();
					int newSize = curSize + grid8.size();
					allPoints.resize(newSize);
					std::copy(grid8.begin(), grid8.end(), allPoints.begin() + curSize);
				}

				double gridX = xMin + (i*perGridLenX);
				double gridY = yMin + (i*perGridLenY);
				double gridZ = zMin + (i*perGridLenZ);

				int totalSize = allPoints.size();
				if (totalSize > K) {
					std::cout << "That happened.\n";
					outData[k * gridResX * gridResY + j * gridResX + i] = 0.0;
				}
				else {
					double val = 0.0;
					double sumOfWeights = 0.0;
					for (auto it = allPoints.begin(); it != allPoints.end(); it++){
						int idx = *it;
						double x = inData[3 * idx + 0];
						double y = inData[3 * idx + 1];
						double z = inData[3 * idx + 2];
						//double alpha = inData[4 * idx + 3];
						double distSq = ((x - gridX)*(x - gridX)) + ((y - gridY)*(y - gridY)) + ((z - gridZ)*(z - gridZ));
						double dist = sqrt(distSq);
						double weight = 1.0 / dist;
						//val += (alpha * weight);
						sumOfWeights += weight;
						//std::cout << idx << "  " << weight << "\n";
					}
					//getchar();
					if (totalSize != 0)
						outData[k * gridResX * gridResY + j * gridResX + i] = sumOfWeights;
					else
						outData[k * gridResX * gridResY + j * gridResX + i] = 0.0;
				}

				progressCounter += 1.0;
			}
		}
		//Display a progress bar
		double tmp = progressCounter / totalProgress;
		progressBarDisplay(tmp);
	}
	std::cout << "\n";
  fwrite (outData , sizeof(double), 64*64*64, fOut);
  fclose(fOut);
}




int main()
{
  //Histogram hist;
  createParticleGrid();
	std::cout << "[INFO] Process Completed....Press Any Key to Exit...";
	getchar();
  return 0;
}

