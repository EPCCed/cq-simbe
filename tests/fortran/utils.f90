module test_utils
implicit none
integer, parameter :: ERROR = -1
integer, parameter :: SUCCESS = 0
integer, parameter :: WARNING = 1

integer :: test_case_number

contains
  subroutine test_header(str)
    implicit none
    character(len=*),intent(in)  :: str
    write(*, *) ''
    write(*, *) '------------------------------------------------------------'
    write(*, *) str
    write(*, *) '------------------------------------------------------------'
  end subroutine test_header

  subroutine assert(condition)
    implicit none
    logical :: condition
    test_case_number = test_case_number + 1
    if (condition) then
      write(*, *) 'Test case: ', test_case_number, ' succeded'
    else 
      write(*, *) 'Test case: ', test_case_number, ' failed'
      stop "Aborting!"
    end if
  end subroutine assert

end module test_utils
