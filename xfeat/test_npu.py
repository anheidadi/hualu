from rknn.api import RKNN
import time
import cv2
rknn = RKNN()
rknn1 = RKNN()

ret = rknn.load_rknn("/home/ubuntu/rvbs/xfeatrknn_ori/weights/xfeat.rknn")  # 替换为你的模型路径
ret1 = rknn1.load_rknn("/home/ubuntu/rvbs/ppyoloe_nms_static_rk3588_quantized.rknn")  # 替换为你的模型路径

if ret != 0:
    print("Load RKNN model failed.")
    exit(ret)
rknn.init_runtime(target="rk3588", eval_mem=True)
rknn1.init_runtime(target="rk3588", eval_mem=True)
#image=cv2.imread("../data/lanepic/oldlane.jpg")
#image=cv2.resize(image,(1280,1280))
#
#rknn.inference(inputs=[image], data_format=['nchw'])
perf_result = rknn.eval_memory()
time.sleep(5)
print(perf_result)
rknn.release()
rknn1.release()

