import os
import subprocess

subprocess.run(["make", "clean"], check=True)
subprocess.run(["time", "make", "backward_autodiff"], check=True)
subprocess.run(["time", "make", "backward_adpp"], check=True)

autodiff_size = os.path.getsize("backward_autodiff")
adpp_size = os.path.getsize("backward_adpp")
print("autodiff binary size: {:.2f} MB".format(autodiff_size/1e6))
print("    adpp binary size: {:.2f} MB".format(adpp_size/1e6))
print("               ratio: {:.2f}".format(autodiff_size/adpp_size))

subprocess.run(["hyperfine", "--warmup", "10", "./backward_autodiff 2.0 4.0", "./backward_adpp 2.0 4.0"], check=True)
