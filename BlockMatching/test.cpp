#include <iostream>
#include <opencv2\opencv.hpp>

#include "Capture.hpp"
#include "Timer.hpp"

int after_pause(int code = 0) {
	system("pause");
	return code;
}

int sum_around_point(cv::Mat &mat, cv::Point &p, int radius) {
	return cv::sum(mat(cv::Rect(p.x - radius, p.y - radius, radius * 2, radius * 2)))[0];
}

template<typename T>
T integral_sum_around_point(cv::Mat &integral, cv::Point &p, int radius) {
	T p0 = integral.at<T>(p.y - radius, p.x - radius);
	T p1 = integral.at<T>(p.y - radius, p.x + radius);
	T p2 = integral.at<T>(p.y + radius, p.x - radius);
	T p3 = integral.at<T>(p.y + radius, p.x + radius);
	return p3 - p2 - p1 + p0;
}

double euclidean_distance(cv::Point &p1, cv::Point &p2) {
	return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

int main(int *argc, int **argv) {
	cv::namedWindow("Render Window", CV_WINDOW_FREERATIO);
	cv::namedWindow("Current", CV_WINDOW_FREERATIO);
	cv::namedWindow("New", CV_WINDOW_FREERATIO);

	Capture capture("../001.b8");
	for (int f_idx = 0; f_idx <= capture.getFrames() - 1; f_idx++) {
		cv::Mat current_frame = capture.getMatFrame(f_idx);
		cv::Mat new_frame = capture.getMatFrame(f_idx + 1);
		cv::Mat canvas(current_frame.size(), current_frame.type(), cv::Scalar(0));

		cv::Mat current_intergral, new_integral;
		cv::integral(current_frame, current_intergral);
		cv::integral(new_frame, new_integral);

		//Set Search properties
		int block_size = 20, block_radius = block_size / 2;
		int search_size = block_size;
		int step_size = 1;

		//Generate all of the search locations
		int indent = block_size + block_radius;
		int width = current_frame.size().width;
		int height = current_frame.size().height;

		//Generate all of the points to search
		int count = ((width - indent) / block_size) * ((height - indent) / block_size);
		std::vector<cv::Point> search_locations(count);

		int temp = 0;
		for (int x = indent; x <= width - indent; x += block_size)
			for (int y = indent; y <= height - indent; y += block_size)
				search_locations[temp++] = cv::Point(x, y);

		int max_sad = block_size * block_size * 256;
		double max_distance = euclidean_distance(cv::Point(0, 0), cv::Point((search_size * 2) + 1, (search_size * 2) + 1));

		Timer t = Timer();
		t.tic();

		//Find where the blocks have moved too in the new frame
		for (int idx = 0; idx < count; idx++) {
			cv::Point search_loaction = search_locations[idx];

			int target_sum = sum_around_point(current_frame, search_loaction, block_radius);
			int best_match = max_sad;
			double closest_match = max_distance;

			cv::Point best_loaction = search_loaction;

			if (target_sum != 0) {
				for (int x = -search_size; x <= search_size; x++) {
					for (int y = -search_size; y <= search_size; y++) {
						cv::Point candidate_loaction = cv::Point(search_loaction.x + x, search_loaction.y + y);

						double candidate_distance = euclidean_distance(search_loaction, candidate_loaction);
						int candidate_sum = sum_around_point(new_frame, candidate_loaction, block_radius);
						int sad = candidate_sum < target_sum ? target_sum - candidate_sum : candidate_sum - target_sum;
						int integral_sum = integral_sum_around_point<int>(new_integral, candidate_loaction, block_radius);

						int sum_of_diff = 0;
						int integral_diff = integral_sum_around_point<int>(cv::Mat(new_integral - current_intergral), candidate_loaction, block_radius);
						for (int i = -block_radius; i <= block_radius; i++) {
							for (int j = -block_radius; j <= block_radius; j++) {
								int current_pixel = current_frame.at<unsigned char>(candidate_loaction.y + j, candidate_loaction.x + i);
								int new_pixel = new_frame.at<unsigned char>(candidate_loaction.y + j, candidate_loaction.x + i);
								sum_of_diff += current_pixel < new_pixel ? new_pixel - current_pixel : current_pixel - new_pixel;
							}
						}

						// TODO: IMPLEMENT INTEGRAL IMAGE SUM OF ABSOLUTE DIFFERENCE FOR INSANE SPEED UP
						// TODO: IMPLEMENT INTEGRAL IMAGE SUM OF ABSOLUTE DIFFERENCE FOR INSANE SPEED UP
						// TODO: IMPLEMENT INTEGRAL IMAGE SUM OF ABSOLUTE DIFFERENCE FOR INSANE SPEED UP
						// TODO: IMPLEMENT INTEGRAL IMAGE SUM OF ABSOLUTE DIFFERENCE FOR INSANE SPEED UP
						// TODO: IMPLEMENT INTEGRAL IMAGE SUM OF ABSOLUTE DIFFERENCE FOR INSANE SPEED UP
						// TODO: IMPLEMENT INTEGRAL IMAGE SUM OF ABSOLUTE DIFFERENCE FOR INSANE SPEED UP
						//http://www.ipol.im/pub/art/2014/57/article_lr.pdf
						std::cout << "SAD: " << sum_of_diff << " Integral SAD: " << integral_diff << " Sum: " << candidate_sum << " Integral: " << integral_sum << std::endl;
						sad = sum_of_diff;//abs(integral_diff); //REPLACE THIS LINE WITH METRIC TO USE (DEFAULT: SAD)

						if (sad < best_match || (sad == best_match && candidate_distance <= closest_match)) {
							best_match = sad;
							closest_match = candidate_distance;
							best_loaction = cv::Point(candidate_loaction.x, candidate_loaction.y);
							std::cout << "Match Found: X = " << candidate_loaction.x << " Y = " << candidate_loaction.y << " SAD = " << sad << " DISTANCE = " << closest_match << std::endl;
						}
					}
				}
			}

			cv::arrowedLine(canvas, search_loaction, best_loaction, cv::Scalar(255));
		}

		//Print info
		std::cout << "Average Frame Rate: " << 1.0 / (t.getElapsed() / 1000000000.0) << " fps" << std::endl;

		cv::imshow("Render Window", canvas);
		cv::imshow("Current", current_frame);
		cv::imshow("New", new_frame);
		cv::waitKey(1);
	}
	std::cout << "Successful Run" << std::endl;

	return 0;
}