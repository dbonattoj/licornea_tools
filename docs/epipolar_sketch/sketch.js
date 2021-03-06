var datasets = [
  "ulb-2017-02-20",
  "ulb-2016-07-26-raw",
  "ulb-2016-07-26-rectified",
  "ulb-2016-05-06-raw",
  "poznan_blocks",
  "bbb_flowers"
];


// config:
var dir;
if(location.search) dir = location.search.substring(1);
else dir = datasets[0];
var left = "L";
var right = "R";
var ext = ".jpg";
/////



var param;
var title;
var left_im, right_im;

// depth map
var have_depth_map;
var left_depth_map_im;
var depth_map_d_near;
var depth_map_d_far;
var depth_map_masked;

// camera parameters
var l_K, l_Rt, r_K, r_Rt, w, h;
var l_K_inv, l_Rt_inv, r_K_inv, r_Rt_inv;
var z_far, z_near;

// user interface
var left_im_position, right_im_position, image_scale;
var point_color = [255, 0, 0];
var line_color = [255, 0, 0];
var line_width = 1;
var depth_slider;
var depth_slider_max = 1000;
var dataset_menu;
var use_depth_map_checkbox, view_depth_map_checkbox;
var line_checkbox, point_checkbox;

// ui state
var left_point = [10.0, 10.0];
var right_point = [10.0, 10.0];
var left_epipolar_line = [0.0, 0.0, 0.0, 0.0];
var right_epipolar_line = [0.0, 0.0, 0.0, 0.0];
var z = 0.0; // orthogonal distance
var slider_d = 0.0; // projected depth (from 0 to 1)


function calculateRightEpipolarLine() {
  var poseTransformation = numeric.dot(l_Rt, r_Rt_inv);
  var t = [poseTransformation[0][3], poseTransformation[1][3], poseTransformation[2][3]];
  var R = numeric.getBlock(poseTransformation, [0, 0], [2, 2]);

  var T = [
    [0.0, -t[2], t[1]],
    [t[2], 0.0, -t[0]],
    [-t[1], t[0], 0.0]
  ];
  var E = numeric.dot(T, R); // essential matrix
  var F = numeric.dot(numeric.transpose(l_K_inv), numeric.dot(E, r_K_inv)); // fundamental matrix

  var x_L = left_point[0], y_L = left_point[1];
  function y_R(x_R) {
    var a = -F[2][2] - F[0][2]*x_L - F[2][0]*x_R - F[0][0]*x_L*x_R - F[1][2]*y_L - F[1][0]*x_R*y_L;
    var b = F[2][1] + F[0][1]*x_L + F[1][1]*y_L;
    return a/b;
  }
  
  var x1 = 0, x2 = w;
  var y1 = y_R(x1), y2 = y_R(x2);

  // put it into screen pixel coordinates for user interface
  x1 = x1*image_scale + right_im_position[0];
  x2 = x2*image_scale + right_im_position[0];
  y1 = y1*image_scale + right_im_position[1];
  y2 = y2*image_scale + right_im_position[1];
  
  right_epipolar_line = [x1, y1, x2, y2];
}


function calculateLeftEpipolarLine() {
  var poseTransformation = numeric.inv(numeric.dot(l_Rt, r_Rt_inv));
  var t = [poseTransformation[0][3], poseTransformation[1][3], poseTransformation[2][3]];
  var R = numeric.getBlock(poseTransformation, [0, 0], [2, 2]);

  var T = [
    [0.0, -t[2], t[1]],
    [t[2], 0.0, -t[0]],
    [-t[1], t[0], 0.0]
  ];
  var E = numeric.dot(T, R); // essential matrix
  var F = numeric.dot(numeric.transpose(r_K_inv), numeric.dot(E, l_K_inv)); // fundamental matrix

  var x_R = right_point[0], y_R = right_point[1];
  function y_L(x_L) {
    var a = -F[2][2] - F[0][2]*x_R - F[2][0]*x_L - F[0][0]*x_R*x_L - F[1][2]*y_R - F[1][0]*x_L*y_R;
    var b = F[2][1] + F[0][1]*x_R + F[1][1]*y_R;
    return a/b;
  }
  
  var x1 = 0, x2 = w;
  var y1 = y_L(x1), y2 = y_L(x2);

  // put it into screen pixel coordinates for user interface
  x1 = x1*image_scale + left_im_position[0];
  x2 = x2*image_scale + left_im_position[0];
  y1 = y1*image_scale + left_im_position[1];
  y2 = y2*image_scale + left_im_position[1];
  
  left_epipolar_line = [x1, y1, x2, y2];
}


function projectionMatrix(K, d_near, d_far) {
  var z_diff = z_far - z_near;
  var offset = ((d_far * z_far) - (d_near * z_near)) / z_diff;
  var factor = ((d_near - d_far) * z_near * z_far) / z_diff;

  return [
    [K[0][0], 0.0, K[0][2], 0.0],
    [0.0, K[1][1], K[1][2], 0.0],
    [0.0, 0.0, offset, factor],
    [0.0, 0.0, 1.0, 0.0]
  ];
}


function calculateCorrespondingPoint() {
  var d = slider_d;
  var d_near = 0.0;
  var d_far = 1.0;

  // projection matrix for left and right camera
  // = 4x4 matrix in homogeneous coordinates (x, y, d, 1.0) = P * v
  //                                        x, y = pixel coordinates in image
  //                                        d = projected depth value
  //                                        v = (x,y,z) 3D coordinates in view coordinate system (with the camera at origin)
  var l_proj = projectionMatrix(l_K, d_near, d_far);
  var r_proj = projectionMatrix(r_K, d_near, d_far);
  
  var l_proj_inv = numeric.inv(l_proj);
  var r_proj_inv = numeric.inv(r_proj);

  // H = homography matrix = r_proj * r_Rt * l_Rt_inv * r_proj_inv
  // = 4x4 matrix, maps (x, y, d, 1.0) on left camera to (x, y, d) on right camera
  var H = numeric.dot(r_proj, numeric.dot(r_Rt, numeric.dot(l_Rt_inv, l_proj_inv)));

  // do matrix-vector multiplication in homographic coordinates
  var x_L = left_point[0], y_L = left_point[1];
  var x_L_h = [x_L, y_L, d, 1.0];
  var x_R_h = numeric.dot(H, x_L_h);
  x_R_h[0] /= x_R_h[3];
  x_R_h[1] /= x_R_h[3];
  
  // resulting pixel coordinates for right point
  right_point = [x_R_h[0], x_R_h[1]];

  // calculate orthogonal distance z: project into view coordinate system, and get z coordinate
  var v_L = numeric.dot(l_proj_inv, x_L_h);
  z = v_L[2] / v_L[3];
}

function preload() {
  loadJSON("datasets/" + dir + "/param.json", function(par) { param = par; });
  left_im = loadImage("datasets/" + dir + "/" + left + ext);
  right_im = loadImage("datasets/" + dir + "/" + right + ext);
}

function convertRt(Rt_) {
  var t_ = [Rt_[0][3], Rt_[1][3], Rt_[2][3]];
  var R_ = numeric.getBlock(Rt_, [0, 0], [2, 2]);

  var R = R_;
  var t = numeric.dot(R, t_);
  var t = [-t[0], -t[1], -t[2]];

  Rt = numeric.identity(4);
  numeric.setBlock(Rt, [0, 0], [2, 2], R);
  for(var i = 0; i < 3; ++i) Rt[i][3] = t[i];
  return Rt;
}

function setup() {
  // read config
  title = param["title"];
  l_K = param[left+"_K"];
  l_Rt = param[left+"_Rt"];
  r_K = param[right+"_K"];
  r_Rt = param[right+"_Rt"];
  w = param["w"];
  h = param["h"];
  image_scale = param["scale"];
  left_im_position = [10, 40];
  right_im_position = [w*image_scale + 20, 40];
  z_near = param["z_near"];
  z_far = param["z_far"];

  // read depth map (if available)
  
  have_depth_map = ("depth_map" in param);
  if(have_depth_map) {
     left_depth_map_im = loadImage("datasets/" + dir + "/" + left + "_depth" + ext);
     depth_map_d_near = param["depth_map"]["d_near"];
     depth_map_d_far = param["depth_map"]["d_far"];
     depth_map_masked = param["depth_map"]["masked"];
  }


  // convert exterinsics from MPEG to standard format if necessary
  if(param["mpeg_extrinsic"]) {
    l_Rt = convertRt(l_Rt);
    r_Rt = convertRt(r_Rt);
  }

  // precompute inverse matrices
  l_K_inv = numeric.inv(l_K);
  l_Rt_inv = numeric.inv(l_Rt);
  r_K_inv = numeric.inv(r_K);
  r_Rt_inv = numeric.inv(r_Rt);
 
  // setup ui
  createCanvas(
    right_im_position[0] + w*image_scale + 10,
    right_im_position[1] + h*image_scale + 10 + 200
  );

  depth_slider = createSlider(0, 1000, 0);
  depth_slider.position(120, h*image_scale + 50);
  depth_slider.input(updateFromSlider);

  dataset_menu = createSelect();
  dataset_menu.position(right_im_position[0], 10);
  for(var i = 0; i < datasets.length; ++i) {
    dataset_menu.option(datasets[i]);
  }
  dataset_menu.value(dir);
  dataset_menu.changed(function() {
    var new_dir = dataset_menu.value();
    window.location = "?" + new_dir;
  });
  
  if(have_depth_map) {
    use_depth_map_checkbox = createCheckbox("use depth map", true);
    use_depth_map_checkbox.position(600, h*image_scale + 50);
    use_depth_map_checkbox.changed(function() {
        updateFromMouse();
    });

    view_depth_map_checkbox = createCheckbox("view depth map", false);
    view_depth_map_checkbox.position(750, h*image_scale + 50);
    view_depth_map_checkbox.changed(function() {
        redraw();
    });
  }
  
  line_checkbox = createCheckbox("epipolar lines", true);
  line_checkbox.position(1000, h*image_scale + 50);
  line_checkbox.changed(redraw);

  point_checkbox = createCheckbox("corresponding point", true);
  point_checkbox.position(1250, h*image_scale + 50);
  point_checkbox.changed(redraw);

  background(255);
  
  // initial state
  z = z_near;
  slider_d = 0.0;
  left_point = [100.0, 100.0];
  calculateCorrespondingPoint(0.0, 0.0, 1.0);
  calculateLeftEpipolarLine();
  calculateRightEpipolarLine();
  
  noLoop();
}


function updateFromSlider() {
  slider_d = depth_slider.value() / depth_slider_max;
  calculateCorrespondingPoint();
  draw();
}


function drawPoint(x, y) {
	var rad = 7;
	strokeWeight(2);
	stroke(point_color);
	line(x - rad, y - rad, x + rad, y + rad);
	line(x + rad, y - rad, x - rad, y + rad);
}


function draw() {
  // clear and redraw...
  clear();

  textSize(23);
  textStyle(BOLD);
  fill(90);
  text(title, 10, 26);

  // images
  var view_depth = (have_depth_map && view_depth_map_checkbox.checked());
  image(view_depth ? left_depth_map_im : left_im, 0, 0, w, h, left_im_position[0], left_im_position[1], w*image_scale, h*image_scale);
  image(right_im, 0, 0, w, h, right_im_position[0], right_im_position[1], w*image_scale, h*image_scale);
  
  // left pt
  var l_x = left_point[0]*image_scale + left_im_position[0];
  var l_y = left_point[1]*image_scale + left_im_position[1];
  drawPoint(l_x, l_y)
  
  if(line_checkbox.checked()) {
	// left epipolar line
    noFill();
    stroke(255, 255, 255, 100);
    strokeWeight(line_width + 3);
    line(left_epipolar_line[0], left_epipolar_line[1], left_epipolar_line[2], left_epipolar_line[3]);
    stroke(line_color);
    strokeWeight(line_width);
    line(left_epipolar_line[0], left_epipolar_line[1], left_epipolar_line[2], left_epipolar_line[3]);

  	
    // right epipolar line
    noFill();
    stroke(255, 255, 255, 100);
    strokeWeight(line_width + 3);
    line(right_epipolar_line[0], right_epipolar_line[1], right_epipolar_line[2], right_epipolar_line[3]);
    stroke(line_color);
    strokeWeight(line_width);
    line(right_epipolar_line[0], right_epipolar_line[1], right_epipolar_line[2], right_epipolar_line[3]);
  }

  if(point_checkbox.checked()) {
    // right pt
    var r_x = right_point[0]*image_scale + right_im_position[0];
    var r_y = right_point[1]*image_scale + right_im_position[1];
    if(r_x > right_im_position[0]) drawPoint(r_x, r_y);
  }

  // rect
  noFill();
  stroke(0);
  strokeWeight(1);
  rect(left_im_position[0], left_im_position[1], w*image_scale, h*image_scale);
  rect(right_im_position[0], right_im_position[1], w*image_scale, h*image_scale);
  
  // slider text
  textStyle(NORMAL);
  fill(0);
  noStroke();
  textSize(12);
  text("projected depth d: " , 10, h*image_scale + 65);
  text("orthogonal distance z = " + floor(z * 100.0)/100.0, 350, h*image_scale + 65);
}

function mouseInBounds() {
	return (mouseX < right_im_position[0] && mouseY < left_im_position[1]+h*image_scale && mouseY >= left_im_position[1]);
}

function updateFromMouse() {
  if(mouseInBounds()) {
    left_point[0] = (mouseX - left_im_position[0]) / image_scale;
    left_point[1] = (mouseY - left_im_position[1]) / image_scale;
  }
  if(have_depth_map && use_depth_map_checkbox.checked()) {
    var depth_pixel = left_depth_map_im.get(left_point[0], left_point[1]);
    var map_d = depth_pixel[0];
    if(map_d != 0 || !depth_map_masked) {
      slider_d = (map_d - depth_map_d_near) / (depth_map_d_far - depth_map_d_near);
      depth_slider.value(slider_d * depth_slider_max);
    }
  }
  calculateCorrespondingPoint();
  calculateLeftEpipolarLine();
  calculateRightEpipolarLine();
  draw();
}


function mousePressed() {
  if(mouseInBounds()) updateFromMouse();
  return true;
}

function mouseDragged() {
  if(mouseInBounds()) updateFromMouse();
  return true;
}
