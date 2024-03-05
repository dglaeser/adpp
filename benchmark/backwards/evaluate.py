import os
import sys
import subprocess
import argparse

parser = argparse.ArgumentParser()
parser.add_argument(
    "-n", "--name",
    required=True,
    help="The name of the benchmark"
)
parser.add_argument(
    "-r", "--reference",
    required=False,
    help="A reference implementation of the benchmark to compare against"
)
parser.add_argument(
    "-a", "--args",
    required=False,
    help="Runtime arguments to be passed to benchmark invocations"
)
args = vars(parser.parse_args())

name = args["name"]
ref_name = args.get("reference")
subprocess.run(["make", "clean"], check=True)
subprocess.run(["time", "make", name], check=True)
if ref_name is not None:
    subprocess.run(["time", "make", ref_name], check=True)

adpp_size = os.path.getsize(name)
print("     adpp binary size: {:.2f} MB".format(adpp_size/1e6))
if ref_name is not None:
    ref_size = os.path.getsize(f"{name}_autodiff")
    print("reference binary size: {:.2f} MB".format(ref_size/1e6))
    print("                ratio: {:.2f}".format(ref_size/adpp_size))

call_args = args.get("args") or ""
if ref_name is not None:
    subprocess.run([
        "hyperfine", "--warmup", "10", f"./{ref_name} {call_args}", f"./{name} {call_args}"
    ], check=True)
else:
    subprocess.run([
        "hyperfine", "--warmup", "10", f"./{name} {call_args}"
    ], check=True)
