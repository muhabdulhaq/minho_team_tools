#include "mainwindow.h"
#include "ui_control_calib.h"

MainWindow::MainWindow(int robot_id, bool real_robot, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    on_max_lin_vel_valueChanged(ui->max_lin_vel->value());
    on_max_ang_vel_valueChanged(ui->max_ang_vel->value());
    ui->action_0->setChecked(true);

    // Setup of ROS
    QString asd = "Control_calib";
    asd.append(QString::number(robot_id));
    asd.append("_");
    asd.append(QString::number(std::time(0)));
    std::stringstream ai_topic;
    std::stringstream ccalib_topic;
    std::stringstream ccalib_sv_topic;
    if(!real_robot){
      // Setup ROS Node and pusblishers/subscribers in SIMULATOR
      ai_topic << "minho_gazebo_robot" << std::to_string(robot_id);
      ccalib_topic << "minho_gazebo_robot" << std::to_string(robot_id);
      ccalib_sv_topic << "minho_gazebo_robot" << std::to_string(robot_id);
    } else {
      // Setup ROS Node and pusblishers/subscribers in REAL ROBOT
      if(robot_id>0){
         // Setup custom master
         QString robot_ip = QString(ROS_MASTER_IP)+QString::number(robot_id)
                            +QString(ROS_MASTER_PORT);
         ROS_WARN("ROS_MASTER_URI: '%s'",robot_ip.toStdString().c_str());
         setenv("ROS_MASTER_URI",robot_ip.toStdString().c_str(),1);
      } else ROS_WARN("ROS_MASTER_URI is localhost");
    }

    ai_topic << "/aiInfo";
    ccalib_topic << "/controlConfig";
    ccalib_sv_topic << "/requestControlConfig";
      
    //Initialize ROS
    int argc = 0;
    ros::init(argc, NULL, asd.toStdString().c_str(),ros::init_options::NoSigintHandler);
    node_ = new ros::NodeHandle();

    ai_pub = node_->advertise<aiInfo>(ai_topic.str().c_str(),100);
    cconfig_pub = node_->advertise<controlConfig>(ccalib_topic.str().c_str(),100);
    controlConfSv = node_->serviceClient<minho_team_ros::requestControlConfig>(ccalib_sv_topic.str().c_str());
    //Request Current Configuration
    requestControlConfig srv; 
    if(controlConfSv.call(srv)){
      ROS_INFO("Retrieved configuration from target robot.");
      ui->spin_Kp_rot->setValue(srv.response.config.Kp_rot);
      ui->spin_Ki_rot->setValue(srv.response.config.Ki_rot);
      ui->spin_Kd_rot->setValue(srv.response.config.Kd_rot);
      ui->spin_Kp_lin->setValue(srv.response.config.Kp_lin);
      ui->spin_Ki_lin->setValue(srv.response.config.Ki_lin);
      ui->spin_Kd_lin->setValue(srv.response.config.Kd_lin);
      ui->max_lin_vel->setValue(srv.response.config.max_linear_velocity);
      ui->lb_max_lin_vel->setText(QString::number(srv.response.config.max_linear_velocity));
      ui->max_ang_vel->setValue(srv.response.config.max_angular_velocity);
      ui->lb_max_ang_vel->setText(QString::number(srv.response.config.max_angular_velocity));
    } else ROS_ERROR("Failed to retrieve configuration from target robot.");

    send_timer = new QTimer();
    connect(send_timer,SIGNAL(timeout()),this,SLOT(sendInfo()));
    send_timer->start(60);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_max_lin_vel_valueChanged(int value)
{
    ui->lb_max_lin_vel->setText(QString::number(value));
    ctrl_config.max_linear_velocity = value;
}

void MainWindow::on_max_ang_vel_valueChanged(int value)
{
    ui->lb_max_ang_vel->setText(QString::number(value));
    ctrl_config.max_angular_velocity = value;
}

void MainWindow::on_bt_voronoi_seg_clicked()
{
    ctrl_config.send_voronoi_seg = !ctrl_config.send_voronoi_seg;
    cconfig_pub.publish(ctrl_config);
}

void MainWindow::on_bt_intersect_seg_clicked()
{
    ctrl_config.send_intersect_seg = !ctrl_config.send_intersect_seg;
    cconfig_pub.publish(ctrl_config);
}

void MainWindow::on_bt_dijk_path_clicked()
{
    ctrl_config.send_dijk_path = !ctrl_config.send_dijk_path;
    cconfig_pub.publish(ctrl_config);
}

void MainWindow::on_bt_dijk_path_obst_circle_clicked()
{
    ctrl_config.send_dijk_path_obst_circle = !ctrl_config.send_dijk_path_obst_circle;
    cconfig_pub.publish(ctrl_config);
}

void MainWindow::on_bt_smooth_path_clicked()
{
    ctrl_config.send_smooth_path = !ctrl_config.send_smooth_path;
    cconfig_pub.publish(ctrl_config);
}

void MainWindow::on_bt_smooth_path_obst_circle_clicked()
{
    ctrl_config.send_smooth_path_obst_circle = !ctrl_config.send_smooth_path_obst_circle;
    cconfig_pub.publish(ctrl_config);
}

void MainWindow::on_bt_path_clicked()
{
    ctrl_config.send_path = !ctrl_config.send_path;
    cconfig_pub.publish(ctrl_config);
}

void MainWindow::on_bt_path_interpolation_clicked()
{
    ctrl_config.send_path_interpolation = !ctrl_config.send_path_interpolation;
    cconfig_pub.publish(ctrl_config);
}

void MainWindow::on_bt_obstacles_circle_clicked()
{
    ctrl_config.send_obstacles_circle = !ctrl_config.send_obstacles_circle;
    cconfig_pub.publish(ctrl_config);
}

void MainWindow::on_spin_Kp_rot_valueChanged(double value)
{
    ctrl_config.Kp_rot = value;
}

void MainWindow::on_spin_Ki_rot_valueChanged(double value)
{
    ctrl_config.Ki_rot = value;
}

void MainWindow::on_spin_Kd_rot_valueChanged(double value)
{
    ctrl_config.Kd_rot = value;
}

void MainWindow::on_spin_Kp_lin_valueChanged(double value)
{
    ctrl_config.Kp_lin = value;
}

void MainWindow::on_spin_Ki_lin_valueChanged(double value)
{
    ctrl_config.Ki_lin = value;
}

void MainWindow::on_spin_Kd_lin_valueChanged(double value)
{
    ctrl_config.Kd_lin = value;
}

void MainWindow::on_targ_x_valueChanged(double value)
{
    ai.target_pose.x = value;
}

void MainWindow::on_targ_y_valueChanged(double value)
{
    ai.target_pose.y = value;
}

void MainWindow::on_targ_theta_valueChanged(double value)
{
    ai.target_pose.z = value;
}

void MainWindow::on_targ_k_x_valueChanged(double value)
{
    ai.target_kick.x = value;
}

void MainWindow::on_targ_k_y_valueChanged(double value)
{
    ai.target_kick.y = value;
}

void MainWindow::on_action_0_clicked(bool state)
{
    if(state) ai.action = 0;
}

void MainWindow::on_action_50_clicked(bool state)
{
    if(state) ai.action = 50;
}

void MainWindow::on_targ_kstr_valueChanged(int value)
{
    ai.target_kick_strength = value;
}

void MainWindow::on_kick_type_chuto_clicked(bool state)
{
    if(state) ai.target_kick_is_pass = false;
}
void MainWindow::on_kick_type_passe_clicked(bool state)
{
    if(state) ai.target_kick_is_pass = true;
}

void MainWindow::on_bt_send_parameters_clicked()
{
    cconfig_pub.publish(ctrl_config);
}

void MainWindow::sendInfo()
{
    ai_pub.publish(ai); 
}

