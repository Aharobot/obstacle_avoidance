#include <ros/ros.h>
#include <pcl_ros/point_cloud.h>
#include <pcl_ros/transforms.h>
#include <pcl/point_types.h>
#include <pcl/filters/passthrough.h>
#include <tf/transform_listener.h>

typedef pcl::PointCloud<pcl::PointXYZ> PointCloud;

class DummyClass {
 public:
  DummyClass(ros::NodeHandle n, ros::NodeHandle np) : ready(false), nh(n), nhp(np) {
    pub = nh.advertise<PointCloud> ("/output", 1);
    sub = nhp.subscribe<PointCloud>("/input", 1, &DummyClass::callback, this);
    ROS_INFO("READY??");
    while (nh.ok()){
      try {
        listener.waitForTransform("/ugv1/base_link", "/ugv1/laser",
        // listener.waitForTransform("/base_link", "/laser",
                                  ros::Time(0), ros::Duration(0.5));
        listener.lookupTransform("/ugv1/base_link", "/ugv1/laser",
        // listener.lookupTransform("/base_link", "/laser",
                                 ros::Time(0), transform);
        if (!ready) {
          ready = true;
          ROS_INFO("READY!!");
        }
      } catch (tf::TransformException ex) {
        ROS_ERROR("%s",ex.what());
        ros::Duration(1.0).sleep();
      }
      ros::spinOnce();
    }
  }

  void callback(const PointCloud::ConstPtr& msg) {
    if (!ready) return;

    PointCloud::Ptr cloud_base_link(new PointCloud);
    //pcl_ros::transformPointCloud ("/ugv1/base_link", *msg, *cloud_base_link, listener);
    pcl_ros::transformPointCloud (*msg, *cloud_base_link, transform);

    cloud_base_link->header.frame_id = "/ugv1/base_link";
    // cloud_base_link->header.frame_id = "/base_link";

    PointCloud::Ptr cloud_filtered(new PointCloud);
    // Create the filtering object
    pcl::PassThrough<pcl::PointXYZ> pass;
    pass.setInputCloud (cloud_base_link);
    pass.setFilterFieldName ("z");
    pass.setFilterLimits(0.3, 1.5);
    //pass.setFilterLimitsNegative (true);
    pass.filter (*cloud_filtered);
    pub.publish(cloud_filtered);
  }

  ros::Publisher pub;
  ros::Subscriber sub;
  tf::StampedTransform transform;
  bool ready;
  ros::NodeHandle nh;
  ros::NodeHandle nhp;
  tf::TransformListener listener;
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "sub_pcl");
  ros::NodeHandle n;
  ros::NodeHandle np("~");
  DummyClass dc(n, np);
}