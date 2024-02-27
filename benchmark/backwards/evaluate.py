import os
import subprocess

subprocess.run(["make", "clean"], check=True)
subprocess.run(["time", "make", "backward_autodiff"], check=True)
subprocess.run(["time", "make", "backward_cppad"], check=True)

autodiff_size = os.path.getsize("backward_autodiff")
cppad_size = os.path.getsize("backward_cppad")
print("autodiff binary size: {:.2f} MB".format(autodiff_size/1e6))
print("   cppad binary size: {:.2f} MB".format(cppad_size/1e6))
print("              ratios: {:.2f}".format(autodiff_size/cppad_size))

subprocess.run(["hyperfine", "--warmup", "10", "./backward_autodiff 2.0 4.0", "./backward_cppad 2.0 4.0"], check=True)
