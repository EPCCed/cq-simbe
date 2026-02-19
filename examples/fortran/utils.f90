module example_utils
use cq
implicit none

contains
  subroutine report_results(cr, NMEASURE, NSHOTS)
    implicit none
    integer(kind=8), value :: NMEASURE
    integer(kind=8), value :: NSHOTS
    integer(kind=2) :: cr(0:NMEASURE * NSHOTS)
    integer(kind=8) :: i, j

    write(*,'(A)') 'Reporting measurement outcomes:'
    do i = 0, NSHOTS-1, 1
        write(*,'(A, I4, A)') 'Shot ', i, ': ' 
      do j = (i) * NMEASURE, (i+1) * NMEASURE - 2, 1
        write(*, '(I4, A)', advance='no') cr(j), ' '
      end do   
      write(*,'(I4, A)') cr((i+1) * NMEASURE - 1), ' '
    end do
  end subroutine report_results

end module
