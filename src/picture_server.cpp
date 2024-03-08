#include <string> 
#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <opencv2/highgui/highgui.hpp>
#include <cv_bridge/cv_bridge.h>
#include "picture_server/SaveImage.h"
#include <sstream>
//Class creation to allow the use of camera callback msg in the service
class PictureServer{
	cv::Mat picture;

public:
	//callback to get camera data through "image_pub" topic
	void imageCallback(const sensor_msgs::ImageConstPtr& msg){
	  try{
		picture = cv_bridge::toCvShare(msg, "bgr8")->image.clone();
	  }
	  catch (cv_bridge::Exception& e){
		ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
	  }
	}
	// service callback that receives "angle" (int representing image name), "path" (path to save image data) and "cmd" (comand confirming if the camera data should be saved). The service response should return a "result" returning 1 if the data was correctly saved
	bool check_and_print(picture_server::SaveImage::Request &req, picture_server::SaveImage::Response &res){
		ROS_INFO("Got new request!");
		if (req.cmd){
			//image name composed by path (finished with "/")+ capture angle+extension
			std::string im_name = req.path + req.num_name+ ".png";
			//checking if the picture has a valid content, otherwise system would failed and stop trying to write the image
			if(!picture.empty()){ 
				if (!cv::imwrite (im_name, picture)){
					res.result = 0;
					std::cout<<"Image can not be saved as '"<<im_name<<"'\n";
				}else{					
					std::cout<<"Image saved in '"<<im_name<<"'\n";
					res.result = 1; // represent success to save the image
					}
			}else{
				res.result = 0;
				ROS_ERROR("Failed to save image\n"); // represent fail to save the image
			}
		}else{
			res.result = 2;// represent that server was called, but image was not requested
		}
	}
};


int main(int argc, char **argv)
{
  PictureServer mi;
  ros::init(argc, argv, "Img_Ctrl_server");
  ros::NodeHandle nh;
  std::string picture_topic;
  image_transport::ImageTransport it(nh);
  ros::ServiceServer service = nh.advertiseService("image_cmd", &PictureServer::check_and_print, &mi);
  nh.param<std::string>("picture_topic", picture_topic, "/strawberry/faces/face_at_center/image_raw");
  image_transport::Subscriber sub = it.subscribe(picture_topic, 1, &PictureServer::imageCallback, &mi);
  ROS_INFO_STREAM(picture_topic);
  ros::spin();
}
