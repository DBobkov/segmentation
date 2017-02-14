/*
  * This file is part of the
  * Chair of Media Technology, Technical University of Munich.
  *
  * Authors:
  *           Sili Chen <sili.chen@tum.de>
  *           Dmytro Bobkov <dmytro.bobkov@tum.de>
  *
  * (C) Copyright 2016-2017 Technical University of Munich
  * All rights reserved.
  */

#include <iostream>

#include <pcl/point_types.h>
#include <pcl/console/parse.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_cloud.h>
#include <pcl/filters/extract_indices.h>

#include "evaluation_computer.h"

void printHelp()
{
    std::cout << "Input as follows: \n";
    std::cout << "--gt_coarse \n";
    std::cout << "--gt_fine \n";
    std::cout << "--pred \n";
}

typedef pcl::PointXYZRGBL PointL;


int main (int argc, char **argv)
{

    pcl::PointCloud<PointL>::Ptr cloud_true_fine(new pcl::PointCloud<PointL>);
    pcl::PointCloud<PointL>::Ptr cloud_true_coarse (new pcl::PointCloud<PointL>);
    pcl::PointCloud<PointL>::Ptr cloud_pred (new pcl::PointCloud<PointL>);


    if ( !pcl::console::find_switch(argc, argv, "--gt_coarse") || !pcl::console::find_switch(argc, argv, "--gt_fine") || !pcl::console::find_switch(argc, argv, "--pred")) {
        return (-1);
    }

    std::string name_gt_fine, name_gt_coarse, name_pred1;
    pcl::console::parse (argc, argv, "--gt_coarse", name_gt_coarse);
    pcl::console::parse (argc, argv, "--gt_fine", name_gt_fine);
    pcl::console::parse (argc, argv, "--pred", name_pred1);

    bool load_coarse_gt = pcl::io::loadPCDFile(name_gt_coarse, *cloud_true_coarse);
    assert(load_coarse_gt>=0);

    bool load_fine_gt = pcl::io::loadPCDFile(name_gt_fine, *cloud_true_fine);
    assert(load_fine_gt>=0);

    bool load_pred = pcl::io::loadPCDFile(name_pred1, *cloud_pred);
    assert(load_pred>=0);

    std::vector<int> indices;
    pcl::removeNaNFromPointCloud(*cloud_true_fine, *cloud_true_fine, indices);
    pcl::removeNaNFromPointCloud(*cloud_true_coarse, *cloud_true_coarse, indices);
    pcl::removeNaNFromPointCloud(*cloud_pred, *cloud_pred, indices);

    // first check that all clouds have same size
    assert(cloud_true_fine->points.size() == cloud_true_coarse->points.size());
    assert(cloud_true_fine->points.size() == cloud_pred->points.size());

    // initialize rand seed
    std::srand (time(NULL));

    // now extract the label
    const int N = cloud_true_fine->points.size();
    std::vector<int> labels_true_fine(N);
    std::vector<int> labels_true_rough(N);
    std::vector<int> labels_pred(N);
    for (size_t i = 0; i < N; i++) {
        labels_true_fine[i] = cloud_true_fine->points[i].label;
        labels_true_rough[i] = cloud_true_coarse->points[i].label;
        labels_pred[i] = cloud_pred->points[i].label;
    }


    //eval
    SegmentationMetric eval_result =  evaluateMultiscale( labels_true_fine, labels_true_rough, labels_pred );
    if( eval_result.status ) {
        std::cout << "***************(" << name_pred1 << ")*******" << std::endl;
        std::cout << "F_os = " << eval_result.oversegmentation_error << std::endl;
        std::cout << "F_us = " << eval_result.undersegmentation_error << std::endl;
    }
    else {
        std::cerr << "Error: cannot get valid score!" << std::endl;
        return -1;
    }

    return 0;
}
