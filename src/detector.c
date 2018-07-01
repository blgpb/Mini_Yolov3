#include "darknet.h"

static int coco_ids[] = {1,2,3,4,5,6,7,8,9,10,11,13,14,15,16,17,18,19,20,21,22,23,24,25,27,28,31,32,33,34,35,36,37,38,39,40,41,42,43,44,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,67,70,72,73,74,75,76,77,78,79,80,81,82,84,85,86,87,88,89,90};

network* net;
char** names; 

static int get_coco_image_id(char *filename)
{
    char *p = strrchr(filename, '/');
    char *c = strrchr(filename, '_');
    if(c) p = c;
    return atoi(p+1);
}

static void print_cocos(FILE *fp, char *image_path, detection *dets, int num_boxes, int classes, int w, int h)
{
    int i, j;
    int image_id = get_coco_image_id(image_path);
    for(i = 0; i < num_boxes; ++i){
        float xmin = dets[i].bbox.x - dets[i].bbox.w/2.;
        float xmax = dets[i].bbox.x + dets[i].bbox.w/2.;
        float ymin = dets[i].bbox.y - dets[i].bbox.h/2.;
        float ymax = dets[i].bbox.y + dets[i].bbox.h/2.;

        if (xmin < 0) xmin = 0;
        if (ymin < 0) ymin = 0;
        if (xmax > w) xmax = w;
        if (ymax > h) ymax = h;

        float bx = xmin;
        float by = ymin;
        float bw = xmax - xmin;
        float bh = ymax - ymin;

        for(j = 0; j < classes; ++j){
            if (dets[i].prob[j]) fprintf(fp, "{\"image_id\":%d, \"category_id\":%d, \"bbox\":[%f, %f, %f, %f], \"score\":%f},\n", image_id, coco_ids[j], bx, by, bw, bh, dets[i].prob[j]);
        }
    }
}

void print_detector_detections(char *id, detection *dets, int total, int classes, int w, int h)
{
    int i, j;
    for(i = 0; i < total; ++i){
        float xmin = dets[i].bbox.x - dets[i].bbox.w/2. + 1;
        float xmax = dets[i].bbox.x + dets[i].bbox.w/2. + 1;
        float ymin = dets[i].bbox.y - dets[i].bbox.h/2. + 1;
        float ymax = dets[i].bbox.y + dets[i].bbox.h/2. + 1;

        if (xmin < 1) xmin = 1;
        if (ymin < 1) ymin = 1;
        if (xmax > w) xmax = w;
        if (ymax > h) ymax = h;

        for(j = 0; j < classes; ++j){
            if (dets[i].prob[j]) printf("%s %f %f %f %f %f\n", id, dets[i].prob[j],
                    xmin, ymin, xmax, ymax);
        }
    }
}

objects get_detections(detection *dets, int num, float thresh, char **names,int classes)
{
    int i,j;
    objects objs;
    objs.cnt = -1;
    objs.objs = calloc(num, sizeof(object));
    for(i = 0; i < num; ++i){
        char labelstr[4096] = {0};
        int class = -1;
        for(j = 0; j < classes; ++j){
            if (dets[i].prob[j] > thresh){
                if (class < 0) {
                    strcat(labelstr, names[j]);
                    class = j;
                } else {
                    strcat(labelstr, ", ");
                    strcat(labelstr, names[j]);
                }
                
                (objs.cnt)++;
                object obj = {names[j],dets[i].prob[j]*100,dets[i].bbox.x,
                            dets[i].bbox.y,dets[i].bbox.w,dets[i].bbox.h};
              
                objs.objs[objs.cnt] = obj;
        }
      }
    }
    objs.cnt++;
    return objs;
} 

objects detect(char* input,char* output)
{
    
    float thresh = 0.5 ,hier_thresh = 0.5 ,nms=0.45;
    int fullscreen = 0;
    
    image **alphabet = load_alphabet();
    image im = load_image_color(input,0,0);
    image sized = letterbox_image(im, net->w, net->h);
    
    layer l = net->layers[net->n-1];

    float *X = sized.data;
        
    network_predict(net, X);
      
    int nboxes = 0;
    detection *dets = get_network_boxes(net, im.w, im.h, thresh, hier_thresh, 0, 1, &nboxes);
  
    if (nms) do_nms_sort(dets, nboxes, l.classes, nms);
    
    objects objs = get_detections(dets, nboxes, thresh, names, l.classes);
    
    draw_detections(im, dets, nboxes, thresh, names, alphabet, l.classes);
    free_detections(dets, nboxes);
        
    save_image(im, output);
  
    free_image(im);
    free_image(sized);

    return objs;
}

void load_net()
{

    char *datacfg = "data/coco.data";
    char *cfg = "data/yolov3-tiny.cfg";
    char *weights = "data/yolov3-tiny.weights";

    list *options = read_data_cfg(datacfg);
    char *name_list = option_find_str(options, "names", "data/names.list");
    names = get_labels(name_list);

    net = load_network(cfg, weights, 0);
    set_batch_network(net, 1);
    srand(2222222);
}
