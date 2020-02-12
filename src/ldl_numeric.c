#include "types.h"
#include "global.h"
#include "constants.h"
#include "pattern.h"

ladel_int ladel_ldl_numeric(ladel_sparse_matrix *Mpp, ladel_symbolics *sym, ladel_factor *LD)
{
    ladel_int row, col, index, ncol = Mpp->ncol, start, index_in_pattern;
    ladel_int *pattern = sym->pattern;
    ladel_double diag_elem, temp, L_elem;
    
    ladel_sparse_matrix *L = LD->L;
    ladel_double *D = LD->D, *Dinv = LD->Dinv;
    ladel_double *rhs = (ladel_double *) ladel_malloc(ncol, sizeof(ladel_double));
    ladel_int *col_pointers = (ladel_int *) ladel_malloc(ncol, sizeof(ladel_int));
    if (!rhs || !col_pointers) 
    {
        ladel_free(rhs);
        ladel_free(col_pointers);
        return FAIL;
    }

    L->p[0] = col_pointers[0] = 0;
    for (index = 1; index < ncol; index++) 
        L->p[index] = col_pointers[index] = sym->col_counts[index-1];
    L->p[ncol] = sym->col_counts[ncol-1];

    for (col = 0; col < ncol; col++)
    {
        rhs[col] = 0;
        for (index = Mpp->p[col]; index < Mpp->p[col+1]; index++) 
            rhs[Mpp->i[index]] = Mpp->x[index];
        diag_elem = rhs[col];
        rhs[col] = 0;

        start = ladel_nonzero_pattern_of_row_in_L(Mpp, sym, col);
        for (index_in_pattern = start; index_in_pattern < ncol; index_in_pattern++)
        {
            row = pattern[index_in_pattern];
            temp = rhs[row] / L->x[L->p[row]];
            L_elem = Dinv[row]*temp; /*L(col,row) = rhs(row) / L(row,row) / D(row,row)*/
            diag_elem -= L_elem*temp; /*D(col, col) -= L(col,row)*D(row,row)*L(col,row)*/
            rhs[row] = 0;
            /* Gaussian elimination */
            
            for (index = L->p[row]+1; index < col_pointers[row]; index++)
                rhs[L->i[index]] -= L->x[index]*temp;
            
            index = col_pointers[row];
            col_pointers[row]++;
            L->i[index] = col;
            L->x[index] = L_elem;
        }
        if (LADEL_ABS(diag_elem) < 1e-14) /*eigenvalue (close to) zero*/
        {
            ladel_free(rhs);
            ladel_free(col_pointers);
            return FAIL;
        }
        D[col] = diag_elem;
        Dinv[col] = 1/diag_elem;
        
        /*TODO: do not store this element in L as we know it is 1*/
        index = col_pointers[col];
        col_pointers[col]++;
        L->i[index] = col;
        L->x[index] = 1;
    }
    ladel_free(rhs);
    ladel_free(col_pointers);
    return SUCCESS;
}