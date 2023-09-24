import random
import numpy as np
import struct
import math


MAX_ALLOWED_NORMAL = 1e240 # ignore the larger ones
MIN_NORMAL = 2.2250738585072014e-308
MAX_SUBNORMAL = 2.224e-308
MIN_SUBNORMAL = 5e-324
MAX_NORMAL_E = 200
MIN_NORMAL_E = -307
MAX_SUBNORMAL_E = -308
MIN_SUBNORMAL_E = -324

N_CASES_PER_SN_CONFIG = 1000


def extract_double(x):
    binary_representation = struct.pack('d', x)
    integer_representation = struct.unpack('Q', binary_representation)[0]
    sign_bit = (integer_representation >> 63) & 1
    mantissa_bits = integer_representation & 0xFFFFFFFFFFFFF
    exponent_bits = (integer_representation >> 52) & 0x7FF
    return sign_bit, mantissa_bits, exponent_bits


def compose_double(sign_bit, mantissa_bits, exponent_bits):
    bits = (sign_bit << 63) | (exponent_bits << 52) | mantissa_bits
    double_value = struct.unpack('d', struct.pack('Q', bits))[0]
    return double_value


def is_subnormal(v):
    v = np.abs(v)
    return v > 0 and v < MAX_SUBNORMAL

def is_normal(x):
    x = np.abs(x)
    return x > MIN_NORMAL and x < MAX_ALLOWED_NORMAL


def fp_classify(x):
    if is_normal(x):
        return 'N'
    if is_subnormal(x):
        return 'S'
    if x == 0:
        return 'O'
    return 'X'


def get_sn_config(output, a, b=None):
    if b is None:
        return fp_classify(output) + fp_classify(a)
    return fp_classify(output) + fp_classify(a) + fp_classify(b)


def random_double_normal(min_exp=1, max_exp=1800):
    sig = 0
    man = np.random.randint(0, 2 ** 52, dtype=np.int64) & 0xFFFFFFFFFFFFF
    exp = np.random.randint(min_exp, max_exp) & 0x7FF
    return compose_double(sig, man, exp)


def random_double_subnormal():
    sig = 0
    man = np.random.randint(0, 2 ** 52, dtype=np.int64) & 0xFFFFFFFFFFFFF
    exp = 0
    return compose_double(sig, man, exp)


def is_vaild(sn_config, output, a, b=None):
     return get_sn_config(output, a, b) == sn_config.upper()


class BaseOp:
    
    def __init__(self) -> None:
        self.name = 'None'

    def check_vaild_and_log(self, sn_config, out, a, b=None):
        rv = is_vaild(sn_config, out, a, b)
        if rv:
            txt = f'{self.name},{sn_config.upper()},{a}' + ('' if b is None else f',{b}')
            print(txt)
        return rv
    

#===========================================================
#   ADD
#===========================================================
class AddOp(BaseOp):
        
    def __init__(self) -> None:
        self.name = 'ADD'

    def add_nnn(self):
        sn_config = 'nnn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_normal()
            b = random_double_normal()
            out = a + b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def add_nsn(self):
        sn_config = 'nsn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            b = random_double_normal()
            out = a + b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def add_nns(self):
        sn_config = 'nns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_normal()
            b = random_double_subnormal()
            out = a + b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def add_nss(self):
        sn_config = 'nss'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            b = random_double_subnormal()
            out = a + b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def add_snn(self):
        sn_config = 'snn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_normal(max_exp=20)
            b = -random_double_normal(max_exp=20)
            out = a + b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def add_sns(self):
        sn_config = 'sns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_normal(max_exp=2)
            b = -random_double_subnormal()
            out = a + b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def add_ssn(self):
        sn_config = 'ssn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = -random_double_subnormal()
            b = random_double_normal(max_exp=2)
            out = a + b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def add_sss(self):
        sn_config = 'sss'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            b = random_double_subnormal()
            out = a + b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def add_run_all(self):
        self.add_nnn()
        self.add_nsn()
        self.add_nns()
        self.add_nss()
        self.add_snn()
        self.add_sns()
        self.add_ssn()
        self.add_sss()


#===========================================================
#   MUL
#===========================================================
class MulOp(BaseOp):

    def __init__(self) -> None:
        self.name = 'MUL'

    def mul_nnn(self):
        sn_config = 'nnn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_normal()
            b = random_double_normal()
            out = a * b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def mul_nsn(self):
        sn_config = 'nsn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            b = random_double_normal()
            out = a * b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def mul_nns(self):
        sn_config = 'nns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_normal()
            b = random_double_subnormal()
            out = a * b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def mul_nss(self):
        # MUL-NSS is an impossible sn-config
        pass

    def mul_snn(self):
        sn_config = 'snn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_normal()
            b = random_double_normal()
            out = a * b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def mul_sns(self):
        sn_config = 'sns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_normal()
            b = random_double_subnormal()
            out = a * b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def mul_ssn(self):
        sn_config = 'ssn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            b = random_double_normal()
            out = a * b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def mul_sss(self):
        sn_config = 'oss'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            b = random_double_subnormal()
            out = a * b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def mul_run_all(self):
        self.mul_nnn()
        self.mul_nsn()
        self.mul_nns()
        self.mul_snn()
        self.mul_sns()
        self.mul_ssn()
        self.mul_sss()


#===========================================================
#   DIV
#===========================================================
class DivOp(BaseOp):

    def __init__(self) -> None:
        self.name = 'DIV'

    def div_nnn(self):
        sn_config = 'nnn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_normal()
            b = random_double_normal()
            out = a / b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def div_nsn(self):
        sn_config = 'nsn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            b = random_double_normal()
            out = a / b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def div_nns(self):
        sn_config = 'nns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_normal()
            b = random_double_subnormal()
            out = a / b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def div_nss(self):
        sn_config = 'nss'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            b = random_double_subnormal()
            out = a / b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def div_snn(self):
        sn_config = 'snn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_normal()
            b = random_double_normal()
            out = a / b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def div_sns(self):
        # DIV-SNS is an impossible sn-config
        pass

    def div_ssn(self):
        sn_config = 'ssn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            b = random_double_normal()
            out = a / b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def div_sss(self):
        sn_config = 'oos'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = 0
            b = random_double_subnormal()
            out = a / b
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def div_run_all(self):
        self.div_nnn()
        self.div_nsn()
        self.div_nns()
        self.div_nss()
        self.div_snn()
        self.div_sns()
        self.div_ssn()
        self.div_sss()


#===========================================================
#   SQRT
#===========================================================
class SqrtOp(BaseOp):

    def __init__(self) -> None:
        self.name = 'SQRT'

    def sqrt_nn(self):
        sn_config = 'nn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_normal()
            out = math.sqrt(a)
            count += self.check_vaild_and_log(sn_config, out, a)

    def sqrt_ns(self):
        sn_config = 'ns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            out = math.sqrt(a)
            count += self.check_vaild_and_log(sn_config, out, a)

    def sqrt_run_all(self):
        self.sqrt_nn()
        self.sqrt_ns()


def main():
    AddOp().add_run_all()
    MulOp().mul_run_all()
    DivOp().div_run_all()
    SqrtOp().sqrt_run_all()


if __name__ == '__main__':
    main()