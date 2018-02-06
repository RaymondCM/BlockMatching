#include <iostream>
#include <opencv2\opencv.hpp>

#include "Capture.hpp"
#include "Timer.hpp"

int main1(int *argc, int **argv) {
	Capture capture("../001.b8");
	cv::Mat current = capture.getMatFrame(0);
	cv::Mat previous = capture.getMatFrame(0);
	cv::Mat previous_intergral, current_intergral;
	cv::Mat canvas = previous.clone();
	int use_integral = true;

	//Get and set properties
	int search_window = 20;
	int search_distance = search_window / 2;
	int edge_size = search_window + search_distance;
	int width = current.size().width;
	int height = current.size().height;

	//Generate all of the points to search
	int count = ((width - edge_size) / search_window) * ((height - edge_size) / search_window);
	cv::Point* search_loactions = new cv::Point[count];

	int temp = 0;
	for (int x = edge_size; x <= width - edge_size; x += search_window)
		for (int y = edge_size; y <= height - edge_size; y += search_window)
			search_loactions[temp++] = cv::Point(x, y);

	Timer t = Timer();
	t.tic();
	for (int i = 1; i < capture.getFrames(); i++) {
		//Check how much the image has moved since last frame
		previous = current.clone();
		current = capture.getMatFrame(i);

		//Calculate integral images
		previous_intergral = current_intergral.clone();
		cv::integral(current, current_intergral);
		canvas = previous.clone();
			
		//Draw points onto the canvas
		for (int i = 0; i < count; i++) {
			cv::circle(canvas, search_loactions[i], 1, (100));
		}

		//Calculate how much each point has moved
		for (int index = 0; index < count; index++) {
			cv::Point current_search_block = search_loactions[index];

			int block_sum = 0;

			//Calculate sum of current block
			if (use_integral) {
				int p0 = current.at<uint8_t>(current_search_block.y - search_distance, current_search_block.x - search_distance);
				int p1 = current.at<uint8_t>(current_search_block.y - search_distance, current_search_block.x + search_distance);
				int p2 = current.at<uint8_t>(current_search_block.y + search_distance, current_search_block.x - search_distance);
				int p3 = current.at<uint8_t>(current_search_block.y + search_distance, current_search_block.x + search_distance);
				block_sum = p3 - p2 - p1 + p0;
			} 
			else {
				for (int x = current_search_block.x - search_distance; x < current_search_block.x + search_distance; x++) {
					for (int y = current_search_block.y - search_distance; y < current_search_block.y + search_distance; y++) {
						block_sum += current.at<uint8_t>(y, x);
					}
				}
			}

			//Create variable for lowest sum and best position
			int lowest_sum = block_sum;
			float lowest_distance = 0;
			cv::Point best_position = current_search_block;

			//Calculate standard deviation
			double average = block_sum / pow(search_window, 2);
			double average_sum = 0;

			for (int x = current_search_block.x - search_distance; x < current_search_block.x + search_distance; x++) {
				for (int y = current_search_block.y - search_distance; y < current_search_block.y + search_distance; y++) {
					average_sum += pow(current.at<uint8_t>(y, x) - average, 2);
				}
			}

			double varience = average_sum / pow(search_window, 2);
			double std_dev = sqrt(varience);

			//Ignore block if standard deviation is low
			if (std_dev > 15 && std_dev < 35) {
				//Loop over blocks in a pixel radius and try to find the best match
				for (int x = -search_distance; x <= search_distance; x++) {
					for (int y = -search_distance; y <= search_distance; y++) {
						//x and y are offsets so add them to the current position
						cv::Point test_location(current_search_block.x + x, current_search_block.y + y);

						int test_sum = 0;
						float test_distance = sqrt((float)(pow(test_location.x - current_search_block.x, 2) + pow(test_location.y - current_search_block.y, 2)));

						//Calculate sum of test location block
						if (use_integral) {
							int p0 = previous.at<uint8_t>(test_location.y - search_distance, test_location.x - search_distance);
							int p1 = previous.at<uint8_t>(test_location.y - search_distance, test_location.x + search_distance);
							int p2 = previous.at<uint8_t>(test_location.y + search_distance, test_location.x - search_distance);
							int p3 = previous.at<uint8_t>(test_location.y + search_distance, test_location.x + search_distance);
							block_sum = p3 - p2 - p1 + p0;
						}
						else {
							for (int x = test_location.x - search_distance; x < test_location.x + search_distance; x++) {
								for (int y = test_location.y - search_distance; y < test_location.y + search_distance; y++) {
									test_sum += previous.at<uint8_t>(y, x);
								}
							}
						}

						test_sum = test_sum < block_sum ? block_sum - test_sum : test_sum - block_sum;

						//Check sum again best one so far and store if it is better
						if (test_sum < lowest_sum || (test_sum == lowest_sum && test_distance <= lowest_distance)) {
							lowest_sum = test_sum;
							best_position.x = test_location.x;
							best_position.y = test_location.y;
							lowest_distance = test_distance;
						}
					}
				}
			}

			//Draw the best point
			cv::arrowedLine(canvas, current_search_block, best_position, (255));
		}

		//Show canvas and wait for input
		cv::imshow("Preview", canvas);
		cv::waitKey(1);
	}

	//Print info
	std::cout << "Average Frame Rate: " << (double) capture.getFrames() / (t.getElapsed() / 1000000000.0) <<  " fps" << std::endl;
	
	//Clear up heap memory
	delete search_loactions;

	system("pause");
	return 0;
}


