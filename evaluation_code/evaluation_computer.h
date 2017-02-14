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
#pragma once
#include <vector>
#include <map>
#include <Eigen/Core>

struct SegmentationMetric {
    float undersegmentation_error;
    float oversegmentation_error;
    bool status;
};

/**
 * @brief buildTruePredsNumMap
 * @param labels_true
 * @param labels_pred
 * @param true_size_map
 * @param pred_size_map
 * @param true_preds_num_map
 * @return
 */
int
buildTruePredsNumMap( const std::vector<int>& labels_true,
                      const std::vector<int>& labels_pred,
                      std::map<int, int>& true_size_map,
                      std::map<int, int>& pred_size_map,
                      std::map<int, std::map<int, int> >& true_preds_num_map );

/**
 * @brief prepareStructureMaps
 * @param labels_true
 * @param labels_pred
 * @param true_size_map
 * @param pred_size_map
 * @param true_max_pred_num_map
 * @return
 */
int
prepareStructureMaps( const std::vector<int> &labels_true,
                      const std::vector<int> &labels_pred,
                      std::map<int, int>& true_size_map,
                      std::map<int, int>& pred_size_map,
                      std::map<int, Eigen::Vector2i >& true_max_pred_num_map );

/**
 * @brief evaluateOSD
 * @param labels_true
 * @param labels_pred
 * @param true_size_map
 * @param pred_size_map
 * @param true_max_pred_num_map
 * @return
 */
SegmentationMetric
evaluateOSD (   const std::vector<int> &labels_true,
                const std::vector<int> &labels_pred,
                std::map<int, int>& true_size_map,
                std::map<int, int>& pred_size_map,
                std::map<int, Eigen::Vector2i >& true_max_pred_num_map );

/**
 * @brief evaluateMultiscale
 * @param labels_true_fine
 * @param labels_true_coarse
 * @param labels_pred
 * @return
 */
SegmentationMetric
evaluateMultiscale (    const std::vector<int>& labels_true_fine,
                        const std::vector<int>& labels_true_coarse,
                        const std::vector<int>& labels_pred );
