#include "helpers.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            BYTE avg = round((image[i][j].rgbtRed + image[i][j].rgbtGreen + image[i][j].rgbtBlue) / 3.0);
            image[i][j].rgbtRed = avg;
            image[i][j].rgbtGreen = avg;
            image[i][j].rgbtBlue = avg;
        }
    }
}

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width / 2; j++)
        {
            RGBTRIPLE temp = image[i][j];
            image[i][j] = image[i][width - j - 1];
            image[i][width - j - 1] = temp;
        }
    }
}

// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE(*copy)[width] = calloc(height, width * sizeof(RGBTRIPLE));
    if (copy == NULL)
    {
        return;
    }

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int sumRed = 0, sumGreen = 0, sumBlue = 0;
            int count = 0;

            for (int row = -1; row <= 1; row++)
            {
                for (int col = -1; col <= 1; col++)
                {
                    int newRow = i + row;
                    int newCol = j + col;

                    if (newRow >= 0 && newRow < height && newCol >= 0 && newCol < width)
                    {
                        sumRed += image[newRow][newCol].rgbtRed;
                        sumGreen += image[newRow][newCol].rgbtGreen;
                        sumBlue += image[newRow][newCol].rgbtBlue;
                        count++;
                    }
                }
            }

            copy[i][j].rgbtRed = round((float) sumRed / count);
            copy[i][j].rgbtGreen = round((float) sumGreen / count);
            copy[i][j].rgbtBlue = round((float) sumBlue / count);
        }
    }

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            image[i][j] = copy[i][j];
        }
    }

    free(copy);
}

// Calculate Gx and Gy values using Sobel kernels
void calculateSobel(int height, int width, RGBTRIPLE image[height][width], int i, int j, int *GxRed, int *GyRed, int *GxGreen, int *GyGreen, int *GxBlue, int *GyBlue)
{
    int GxKernel[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int GyKernel[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    *GxRed = 0;
    *GyRed = 0;
    *GxGreen = 0;
    *GyGreen = 0;
    *GxBlue = 0;
    *GyBlue = 0;

    for (int row = -1; row <= 1; row++)
    {
        for (int col = -1; col <= 1; col++)
        {
            int newRow = i + row;
            int newCol = j + col;

            // Handle edge cases by using black pixels
            if (newRow < 0 || newRow >= height || newCol < 0 || newCol >= width)
            {
                continue;
            }

            *GxRed += image[newRow][newCol].rgbtRed * GxKernel[row + 1][col + 1];
            *GyRed += image[newRow][newCol].rgbtRed * GyKernel[row + 1][col + 1];

            *GxGreen += image[newRow][newCol].rgbtGreen * GxKernel[row + 1][col + 1];
            *GyGreen += image[newRow][newCol].rgbtGreen * GyKernel[row + 1][col + 1];

            *GxBlue += image[newRow][newCol].rgbtBlue * GxKernel[row + 1][col + 1];
            *GyBlue += image[newRow][newCol].rgbtBlue * GyKernel[row + 1][col + 1];
        }
    }
}


// Calculate final edge value
BYTE calculateEdgeValue(int Gx, int Gy)
{
    return (BYTE) round(sqrt(Gx * Gx + Gy * Gy));
}

// Apply edge detection using Sobel operator
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE(*copy)[width] = calloc(height, width * sizeof(RGBTRIPLE));
    if (copy == NULL)
    {
        return;
    }

    // Iterate over every pixel in the image
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int GxRed, GyRed, GxGreen, GyGreen, GxBlue, GyBlue;
            calculateSobel(height, width, image, i, j, &GxRed, &GyRed, &GxGreen, &GyGreen, &GxBlue, &GyBlue);

            // Combine Gx and Gy for each color channel
            int edgeRed = round(sqrt(GxRed * GxRed + GyRed * GyRed));
            int edgeGreen = round(sqrt(GxGreen * GxGreen + GyGreen * GyGreen));
            int edgeBlue = round(sqrt(GxBlue * GxBlue + GyBlue * GyBlue));

            // Cap the values at 255
            edgeRed = edgeRed > 255 ? 255 : edgeRed;
            edgeGreen = edgeGreen > 255 ? 255 : edgeGreen;
            edgeBlue = edgeBlue > 255 ? 255 : edgeBlue;

            // Assign the calculated edge values to the copy image
            copy[i][j].rgbtRed = edgeRed;
            copy[i][j].rgbtGreen = edgeGreen;
            copy[i][j].rgbtBlue = edgeBlue;
        }
    }

    // Copy the edge-detected image back to the original image
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            image[i][j] = copy[i][j];
        }
    }

    // Free the allocated memory for the copy image
    free(copy);
}
