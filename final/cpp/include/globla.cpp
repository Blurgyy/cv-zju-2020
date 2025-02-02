#include "globla.hpp"

std::tuple<CamConf, CamConf> read_cam(std::string const &filename) {
    CamConf       lret, rret;
    std::ifstream from{filename};
    if (from.fail()) {
        eprintf("Failed opening file %s\n", filename.c_str());
    }
    /* Check if reading is successful */
    int six = 0;
    for (std::string line; std::getline(from, line);) {
        std::istringstream in{line};
        std::string        token;
        in >> token;
        if (token == "fx") {
            flt fx;
            in >> fx;
            lret.fx = rret.fx = fx;
            ++six;
        } else if (token == "cx") {
            flt cx;
            in >> cx;
            lret.cx = rret.cx = cx;
            ++six;
        } else if (token == "cy") {
            flt cy;
            in >> cy;
            lret.cy = rret.cy = cy;
            ++six;
        } else if (token == "fy") {
            flt fy;
            in >> fy;
            lret.fy = rret.fy = fy;
            ++six;
        } else if (token == "left" || token == "Left") {
            for (int i = 0; i < 3; ++i) {
                std::getline(from, line);
                in = std::istringstream{line};
                in >> lret.rot[i][0] >> lret.rot[i][1] >> lret.rot[i][2] >>
                    lret.trans[i];
            }
            ++six;
        } else if (token == "right" || token == "Right") {
            for (int i = 0; i < 3; ++i) {
                std::getline(from, line);
                in = std::istringstream{line};
                in >> rret.rot[i][0] >> rret.rot[i][1] >> rret.rot[i][2] >>
                    rret.trans[i];
            }
            ++six;
        }
    }
    if (six != 6) {
        dump(lret);
        dump(rret);
        eprintf("Failed reading camera configs\n");
    }
    return {lret, rret};
}

MiscConf read_calib(std::string const &filename) {
    MiscConf ret;
    ret.ndisp = 0;
    std::ifstream from{filename};
    if (from.fail()) {
        eprintf("Failed opening file %s\n", filename.c_str());
    }
    flt dummy;
    for (std::string line; std::getline(from, line);) {
        for (int i = 0; i < line.length(); ++i) {
            if (line[i] == '=' || line[i] == '[' || line[i] == ']' ||
                line[i] == ';' || line[i] == ',') {
                line[i] = ' ';
            }
        }
        std::istringstream in{line};
        std::string        token;
        in >> token;
        if (token == "cam0") {
            in >> ret.left.fx >> dummy >> ret.left.cx >> dummy >>
                ret.left.fy >> ret.left.cy;
        } else if (token == "cam1") {
            in >> ret.right.fx >> dummy >> ret.right.cx >> dummy >>
                ret.right.fy >> ret.right.cy;
        } else if (token == "doffs") {
            in >> ret.doffs;
        } else if (token == "baseline") {
            in >> ret.baseline;
        } else if (token == "width") {
            in >> ret.width;
        } else if (token == "height") {
            in >> ret.height;
        } else if (token == "ndisp") {
            in >> ret.ndisp;
        } else if (token == "ndisp") {
            in >> ret.ndisp;
        } else if (token == "isint") {
            in >> ret.isint;
        } else if (token == "vmin") {
            in >> ret.vmin;
        } else if (token == "vmax") {
            in >> ret.vmax;
        } else if (token == "dyavg") {
            in >> ret.dyavg;
        } else if (token == "dymax") {
            in >> ret.dymax;
        }
    }
    return ret;
}

cv::Mat map_back(std::vector<ppp> const &pixel_map, int const &rows,
                 int const &cols, cv::Mat const &disp) {
    if (disp.type() != CV_32SC1) {
        eprintf("Expected disparity map type is CV_32SC1 (%d), got %d\n",
                disp.type());
    }
    if (pixel_map.size() == 0) {
        return disp;
    }
    int     drows = disp.rows;
    int     dcols = disp.cols;
    cv::Mat ret   = cv::Mat(rows, cols, CV_32SC1, -1);

    for (ppp const &item : pixel_map) {
        SpatialPoint dep_p = item.second;
        int          dep_x = dep_p.pos.x;
        int          dep_y = dep_p.pos.y;
        if (0 <= dep_x && dep_x < dcols && //
            0 <= dep_y && dep_y < drows) {
            SpatialPoint ori_p        = item.first;
            int          ori_x        = ori_p.pos.x;
            int          ori_y        = ori_p.pos.y;
            flt          depth        = disp.at<int>(dep_y, dep_x);
            ret.at<int>(ori_y, ori_x) = depth;
        }
    }

    return ret;
}

cv::Mat visualize(cv::Mat const &input, flt const &gamma) {
    if (input.channels() != 1) {
        eprintf("More than 1 channels encountered when attempting to "
                "normalize image\n");
    }
    if (input.type() != CV_32SC1) {
        eprintf("Expected disparity map type is CV_32SC1 (%d), got %d\n",
                input.type());
    }

    int rows = input.rows;
    int cols = input.cols;

    cv::Mat ret(rows, cols, CV_8UC1);

    flt maxd = std::numeric_limits<flt>::lowest();
    flt mind = std::numeric_limits<flt>::max();

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            flt const &value = input.at<int>(y, x);
            maxd             = std::max<flt>(maxd, value);
            mind             = std::min<flt>(mind, value);
        }
    }

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            flt const &ivalue     = input.at<int>(y, x);
            flt        value      = (ivalue - mind) / (maxd - mind);
            value                 = std::pow(value, gamma);
            value                 = value * 256 - 0.5;
            ret.at<uint8_t>(y, x) = static_cast<uint8_t>(value);
        }
    }

    cv::applyColorMap(ret, ret, cv::COLORMAP_JET);
    return ret;
}

void get_matches(cv::Mat const &limg, cv::Mat const &rimg,
                 std::vector<cv::KeyPoint> &kp1,
                 std::vector<cv::KeyPoint> &kp2,
                 std::vector<cv::DMatch> &  matches) {
    kp1.clear();
    kp2.clear();
    cv::Mat          desc1, desc2;
    cv::Ptr<cv::ORB> orb =
        cv::ORB::create(500, 1.2, 8, 31, 0, 2, cv::ORB::HARRIS_SCORE, 31, 20);

    /* 1. Detect key points */
    orb->detect(limg, kp1);
    orb->detect(rimg, kp2);

    /* 2. Compute descriptors */
    orb->compute(limg, kp1, desc1);
    orb->compute(rimg, kp2, desc2);

    /* 3. Match descriptors with hamming distance */
    std::vector<cv::DMatch> all_matches;
    cv::BFMatcher           matcher(cv::NORM_HAMMING);
    matcher.match(desc1, desc2, all_matches);

    /* 4. Filter keypoints */
    flt maxd = std::numeric_limits<flt>::lowest();
    flt mind = std::numeric_limits<flt>::max();
    for (int y = 0; y < desc1.rows; ++y) {
        mind = std::min<flt>(mind, all_matches[y].distance);
        maxd = std::max<flt>(maxd, all_matches[y].distance);
    }

    for (int y = 0; y < desc1.rows; ++y) {
        if (all_matches[y].distance <= std::max<flt>(2 * mind, 30)) {
            matches.push_back(all_matches[y]);
        }
    }

    cv::Mat img;
    cv::drawMatches(limg, kp1, rimg, kp2, matches, img);
    cv::imwrite("matched.png", img);
}

cv::Vec3b lerp(cv::Vec3b const &a, cv::Vec3b const &b, flt const &t) {
    return cv::Vec3b{
        static_cast<unsigned char>(a[0] * (1 - t) + b[0] * t),
        static_cast<unsigned char>(a[1] * (1 - t) + b[1] * t),
        static_cast<unsigned char>(a[2] * (1 - t) + b[2] * t),
    };
}
int lerp(int const &a, int const &b, flt const &t) {
    return static_cast<int>(a * (1 - t) + b * t);
}

void interpolate(cv::Mat &img) {
    if (img.type() != CV_8UC3) {
        eprintf("\n");
    }
    int dx[] = {-1, 1};
    int dy[] = {-1, 1};
    int rows = img.rows;
    int cols = img.cols;
    for (int y = 1; y < rows - 1; ++y) {
        for (int x = 1; x < cols - 1; ++x) {
            cv::Vec3b &elem = img.at<cv::Vec3b>(y, x);
            if (elem == cv::Vec3b{0, 0, 0}) {
                int cnt = 0;
                flt b = 0, g = 0, r = 0;
                for (int xoff : dx) {
                    for (int yoff : dy) {
                        cv::Vec3b nei = img.at<cv::Vec3b>(y + yoff, x + xoff);
                        if (nei == cv::Vec3b(0, 0, 0)) {
                            continue;
                        }
                        ++cnt;
                        b += nei[0];
                        g += nei[1];
                        r += nei[2];
                    }
                }
                b /= cnt, g /= cnt, r /= cnt;
                elem = cv::Vec3b(b, g, r);
            }
        }
    }
}

// Author: Blurgy <gy@blurgy.xyz>
// Date:   Mar 07 2021, 16:15 [CST]
