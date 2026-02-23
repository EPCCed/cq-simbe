program test_qasm_gates
use cq
#include "cqf.h"

use test_utils
implicit none

call test_simple_gates()
call test_rotation_gates()
call test_control_gates()
call test_control_rotation_gates()
call test_multi_control_gates()

contains
subroutine test_simple_gates()
end subroutine

subroutine test_rotation_gates()
end subroutine

subroutine test_control_gates()
end subroutine

subroutine test_control_rotation_gates()
end subroutine

subroutine test_multi_control_gates()
end subroutine

end
