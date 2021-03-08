#pragma once

#include "globla.hpp"

void pose_estimation(std::vector<cv::KeyPoint> const &kp1,
                     std::vector<cv::KeyPoint> const &kp2,
                     std::vector<cv::DMatch> const &matches, mat3 const &K,
                     mat3 &R, vec3 &t);

/* Sum of absolute difference (SAD)
 * @param `{l,r}img` **Rectified** stereo images.
 * @param `wr` Window radius, window size is: `wr` * 2 + 1
 * @param `fx` **Effective** focal length on `x` axis of the 2 given rectified
 *        stereo images.
 * @return Depth map.
 */
cv::Mat SAD(cv::Mat const &limg, cv::Mat const &rimg, int const &wr,
            MiscConf const &conf);

// Author: Blurgy <gy@blurgy.xyz>
// Date:   Feb 27 2021, 17:20 [CST]