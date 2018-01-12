#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>

class Capture {
private:
	struct dicom_header {
		int type;
		int frames;
		int width;
		int height;
		int sample_size;
		int roi[8]; //ULX,ULY,URX,URY,BRX,BRY,BLX,BRY
		int probe;
		int transmit_freq;
		int sample_freq;
		int data_rate;
		int line_density;
		int extra_info;
	};

	const char *file_path;
	_iobuf *file;
	dicom_header header;
	char *data;
	int frame_size, frame_index = 0, data_type = CV_8UC1;
public:
	Capture(std::string path) : file_path(path.c_str()), file(fopen(file_path, "rb")) {
		if (!file)
			throw std::runtime_error("Error opening file: " + path);

		fread(&header, sizeof(header), 1, file);
		std::cout << "Info:\n" << "  Width: " << header.width << "\n  Height: " << header.height << "\n  Sample Size: " << header.sample_size <<
			"\n  Frame Count: " << header.frames << "\n  Frame Rate: " << header.data_rate << std::endl;

		frame_size = header.width * header.height * (header.sample_size / 8);

		data = new char[frame_size * header.frames];
		fread(data, frame_size * header.frames, 1, file);
		fclose(file);
	}

	int getFrames() { return header.frames; };

	int getHeight() { return header.height; };

	int getWidth() { return header.width; };

	cv::Size getSize() { return cv::Size(header.width, header.height); };

	int getType() { return data_type; };

	char * getRawFrame(int index) {
		char *frame = new char[frame_size];
		memcpy(frame, data + (frame_size * index), frame_size);
		return frame;
	}

	cv::Mat getMatFrame(int index) {
		frame_index = index;
		return cv::Mat(getSize(), data_type, &data[(frame_size * index)]);
	}

	cv::Mat& operator >> (cv::Mat& in)
	{
		in = getMatFrame(frame_index);
		this->frame_index++;
		return in;
	};
};