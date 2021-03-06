#ifndef YOLO_LAYER_H
#define YOLO_LAYER_H

#include "darknet.h"
#include "layer.h"
#include "network.h"

layer make_yolo_layer(int batch, int w, int h, int n, int total, int *mask, int classes);
void forward_yolo_layer(const layer l, network net);
int yolo_num_detections(layer l, float thresh);

#endif
