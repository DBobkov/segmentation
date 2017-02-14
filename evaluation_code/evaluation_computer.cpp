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

#include "evaluation_computer.h"
#include <iostream>

int
buildTruePredsNumMap( const std::vector<int>& labels_true,
                      const std::vector<int>& labels_pred,
                      std::map<int, int>& true_size_map,
                      std::map<int, int>& pred_size_map,
                      std::map<int, std::map<int, int> >& true_preds_num_map )
{
    int num_all = 0;
    for (size_t i = 0; i < labels_true.size(); i++) {
        const int label_true = labels_true[i];
        const int label_pred = labels_pred[i];
        if( label_true<=0 ) { // skip 0 label
            continue;
        }

        num_all ++;
        if( true_size_map.count(label_true) == 0 ) {
            true_size_map[label_true] = 1;
        }
        else {
            true_size_map[label_true] ++;
        }

        if( true_preds_num_map.count(label_true) == 0 ) {
            std::map<int,int> temp_map;
            temp_map[label_pred] = 1;
            true_preds_num_map[label_true] = temp_map;
        }
        else {
            std::map<int, int> current_map = true_preds_num_map[label_true];
            if( current_map.count(label_pred) == 0 ) {
                current_map[label_pred] = 1;
            }
            else {
                current_map[label_pred]++;
            }
            true_preds_num_map[label_true] = current_map;
        }

        if( pred_size_map.count(label_pred) == 0 ) {
            pred_size_map[label_pred] = 1;
        }
        else {
            pred_size_map[label_pred]++;
        }

    }
    return num_all;
}


int
prepareStructureMaps( const std::vector<int> &labels_true,
                      const std::vector<int> &labels_pred,
                      std::map<int, int>& true_size_map,
                      std::map<int, int>& pred_size_map,
                      std::map<int, Eigen::Vector2i >& true_max_pred_num_map )
{
    std::map<int, std::map<int, int> > true_preds_num_map;
    //key: true label; value map: key: pred label related to current true label, value: #points of this pred label belonging to current true label
    // build true_preds_num_map , pred_size_map and true_size_map
    int num_all = buildTruePredsNumMap( labels_true, labels_pred, true_size_map, pred_size_map, true_preds_num_map ); // count number of points with label > 0

    /// Following function does NOT use 0.5 criteria
    for( std::map<int, std::map<int, int> >::const_iterator mit = true_preds_num_map.cbegin(); mit != true_preds_num_map.cend(); mit++ )
    {
        int max_pred_label_num = 0;
        int max_pred_label = 0;
        const std::map<int, int>& current_map = mit->second;
        for (std::map<int, int>::const_iterator mmit = current_map.cbegin(); mmit != current_map.cend(); mmit++) {
            if( mmit->second > max_pred_label_num) {
                max_pred_label_num = mmit->second;
                max_pred_label = mmit->first;
            }
        }
        if (max_pred_label_num > 0) {
            Eigen::Vector2i current_vec(max_pred_label, max_pred_label_num);
            true_max_pred_num_map[mit->first] = current_vec;
        }
        else {
            std::cerr << "Error: there is no pred label for this group!!!" << std::endl;
            return -1;
        }
    }
    return num_all;
}



SegmentationMetric
evaluateOSD (   const std::vector<int> &labels_true,
                const std::vector<int> &labels_pred,
                std::map<int, int>& true_size_map,
                std::map<int, int>& pred_size_map,
                std::map<int, Eigen::Vector2i >& true_max_pred_num_map )
{
    SegmentationMetric eval_result;

    int num_all = prepareStructureMaps( labels_true, labels_pred, true_size_map, pred_size_map, true_max_pred_num_map );

    assert(num_all>0);

    // calculate over-segmentation error and under-segmentation error
    float F_os = 0;
    float F_us = 0;
    for (std::map<int, Eigen::Vector2i >::const_iterator mit = true_max_pred_num_map.cbegin(); mit != true_max_pred_num_map.cend(); mit++)
    {
        const Eigen::Vector2i& current_vec = mit->second;
        if( current_vec[0] == -1 || current_vec[1] <= 0 ) {
            continue;
        }

        F_os += current_vec[1];
        if( pred_size_map.count(current_vec[0])>0 )
        {
            if (pred_size_map[current_vec[0]] - current_vec[1] >= 0) {
                F_us += (pred_size_map[current_vec[0]] - current_vec[1]);
            }
            else
            {
                std::cerr << "Error: pred_size_map[current_vec[0]] - current_vec[1] < 0!!!" << std::endl;
                eval_result.status = false;
                return eval_result;
            }
        }
        else
        {
            std::cerr << "Error: error in pred_size_map!" << std::endl;
            eval_result.status = false;
            return eval_result;
        }
    }

    F_os = 1.0f - F_os / (float) num_all;
    eval_result.oversegmentation_error = F_os;
    eval_result.undersegmentation_error = F_us;
    eval_result.status = true;

    return eval_result;

}


SegmentationMetric
evaluateMultiscale (    const std::vector<int>& labels_true_fine,
                        const std::vector<int>& labels_true_coarse,
                        const std::vector<int>& labels_pred )
{
    SegmentationMetric eval_result;

    std::map<int, int> true_fine_size_map, true_rough_size_map;
    std::map<int, Eigen::Vector2i > true_fine_max_true_rough_num_map;


    int num_all1 = prepareStructureMaps( labels_true_fine, labels_true_coarse, true_fine_size_map, true_rough_size_map, true_fine_max_true_rough_num_map );
    assert(num_all1>0 && "Error: in evaluation of fine and rough!!!");

    std::map<int, int> true_fine_size_map_dummy, pred_size_map;
    std::map<int, Eigen::Vector2i > true_fine_max_pred_num_map;
    int num_all2 = prepareStructureMaps( labels_true_fine, labels_pred, true_fine_size_map_dummy, pred_size_map, true_fine_max_pred_num_map );
    assert(num_all2>0 && "Error: in evaluation of fine and pred!!!");

    int num_all = 0; //number of objects points
    for( const auto & m : true_fine_size_map ) {
        num_all += m.second;
    }

    std::map<int, Eigen::Vector2i > scale_pred_num_map;
    std::map<int, int> scale_size_map;
    for( const auto& mit : true_fine_max_pred_num_map ) {
        if( mit.second[0] != -1 ) {
            continue;
        }

        scale_pred_num_map[mit.first] = mit.second;
        assert(true_fine_size_map.count(mit.first)>0 && "Error: cannot find scale label in true_fine_size_map !!!");

        scale_size_map[mit.first] = true_fine_size_map[mit.first];


    }

    for( std::map<int, Eigen::Vector2i >::iterator mit = true_fine_max_pred_num_map.begin(); mit!=true_fine_max_pred_num_map.end(); mit++ ) {
        if( mit->second[0] == -1) {
            continue;
        }

        Eigen::Vector2i current_vec = mit->second;
        for( auto mit_temp = std::next(mit); mit_temp!= true_fine_max_pred_num_map.end(); mit_temp++ ) {
            Eigen::Vector2i current_vec_temp = mit_temp->second;
            bool flag_ok = (mit->first != mit_temp->first &&  // two different fine label
                    current_vec[0] == current_vec_temp[0] &&  // the same pred label , and != -1
                    true_fine_max_true_rough_num_map.count(mit->first) >0 &&
                    true_fine_max_true_rough_num_map.count(mit_temp->first) > 0);
            if( !flag_ok ) {
                continue;
            }

            Eigen::Vector2i current_rough_vec = true_fine_max_true_rough_num_map[mit->first];
            Eigen::Vector2i current_rough_vec_temp = true_fine_max_true_rough_num_map[mit_temp->first];
            bool fine_labels_have_same_coarse_label = (current_rough_vec[0] != -1 && current_rough_vec[0] == current_rough_vec_temp[0]);
            if( !fine_labels_have_same_coarse_label ) {
                continue;
            }

            current_vec[1] += current_vec_temp[1];
            current_vec_temp << -1, 0;

            mit->second = current_vec;
            mit_temp->second = current_vec_temp;

            bool true_fine = (true_fine_size_map.count(mit->first)>0 && true_fine_size_map.count(mit_temp->first) >0 );
            assert(true_fine && "Error: in true_fine_size_map !!!");

            true_fine_size_map[mit->first] += true_fine_size_map[mit_temp->first];
            true_fine_size_map[mit_temp->first] = 0;
        }
        scale_size_map[mit->first] = true_fine_size_map[mit->first];
        scale_pred_num_map[mit->first] = current_vec;
    }


    // calculate over-segmentation error and under-segmentation error
    float F_os = 0;
    float F_us = 0;
    for( const auto& mit : scale_pred_num_map ) {
        if( mit.second[0]==-1 || mit.second[1]<=0 ) {
            continue;
        }
        F_os += mit.second[1];

        assert(pred_size_map.count(mit.second[0])>0 && "Error: error in pred_size_map!");

        int diff = pred_size_map[mit.second[0]] - mit.second[1];
        assert(diff>=0 && "Error: pred_size_map[current_vec[0]] - current_vec[1] < 0!!!");


        F_us += (pred_size_map[mit.second[0]] - mit.second[1]);
    }

    int num_all_verify = 0;
    for( const auto& mit : true_fine_size_map ) {
        num_all_verify += mit.second;
    }

    assert( num_all == num_all_verify && "Error: num_all != num_all_verify !!!");

    // normalize by the number of points
    F_os = 1.0f - F_os / (float) num_all;
    F_us /= (float) num_all;

    eval_result.status = true;
    eval_result.oversegmentation_error = F_os;
    eval_result.undersegmentation_error = F_us;

    return eval_result;
}
