
#pragma once

#ifndef __GFE_KuhnMunkres_h__
#define __GFE_KuhnMunkres_h__

#include <GFE/GFE_KuhnMunkres.h>

#include <GFE/GFE_GeoFilter.h>


#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <vector>
#include <algorithm>



// @see Tutorial on Implementation of Munkres' Assignment Algorithm
//      Robert Pilgram, Murray State University

namespace detail
{
    template<typename T>
    struct MunkresData
    {
        enum MunkresState : uint8_t { NONE, STAR, PRIME };
        using edge = std::pair<uint32, uint32>;
  
        uint32 n_rows_ = 0;
        uint32 n_cols_ = 0;
        uint32 side_   = 0;
        std::vector<T> data;
        std::vector<MunkresState> marks;
        std::vector<bool> row_mask;
        std::vector<bool> col_mask;
  
        // ------------------------------------------------------------- Construction
        //
        MunkresData(const uint32 n_rows,
                    const uint32 n_cols,
                    std::function<T(uint32 r, uint32 c)> edge_cost) noexcept
            : n_rows_(n_rows)
            , n_cols_(n_cols)
            , side_(std::max(n_rows, n_cols))
            , data(side_ * side_)
            , marks(side_ * side_)
            , row_mask(side_)
            , col_mask(side_)
        {
           assert(n_rows_ > 0);
           assert(n_cols_ > 0);
  
           // Populate weight matrix... keep track of maximum for next step
           T max_val = std::numeric_limits<T>::lowest();
           for (auto r = 0u; r < n_rows; ++r)
              for (auto c = 0u; c < n_cols; ++c) {
                 auto val = edge_cost(r, c);
                 C(r, c)  = val;
                 if (max_val < val) max_val = val;
              }
  
           // The weight matrix is always square... fill in the empty
           // spots with 'max-val'
           for (auto r = n_rows; r < side(); ++r)
              for (auto c = n_cols; c < side(); ++c) C(r, c) = max_val;
           for (auto c = n_cols; c < side(); ++c)
              for (auto r = n_rows; r < side(); ++r) C(r, c) = max_val;
  
           // Subtract the minimum from every row and column, which
           // ensures that every row and column has a '0'
           subtract_min_from_all_rows_cols();
  
           // Set up marks
           std::fill(begin(marks), end(marks), MunkresState::NONE);
           std::fill(begin(row_mask), end(row_mask), false);
           std::fill(begin(col_mask), end(col_mask), false);
  
           { // ensure evey element is finite
              auto ii = std::find_if(
                  cbegin(data), cend(data), [](auto x) { return !std::isfinite(x); });
              if (ii != cend(data)) {
                 std::cerr << "precondition failed: non-finite edge cost" << std::endl;
                 assert(false);
              }
           }
        }
  
        // ---------------------------------------------------------- Getters/Setters
        //
        // Costs
        T& C(int r, int c) noexcept { return data[r * side_ + c]; }
        const T& C(int r, int c) const noexcept { return data[r * side_ + c]; }
  
        // Marks
        MunkresState& M(int r, int c) noexcept { return marks[r * side_ + c]; }
        const MunkresState& M(int r, int c) const noexcept { return marks[r * side_ + c]; }
  
        void cover_row(int r) noexcept { row_mask[r] = true; }
        void cover_col(int c) noexcept { col_mask[c] = true; }
        void uncover_row(int r) noexcept { row_mask[r] = false; }
        void uncover_col(int c) noexcept { col_mask[c] = false; }
        bool is_row_covered(int r) const noexcept { return row_mask[r]; }
        bool is_col_covered(int c) const noexcept { return col_mask[c]; }
  
        uint32 original_cols() const noexcept { return n_cols_; }
        uint32 original_rows() const noexcept { return n_rows_; }
        uint32 side() const noexcept { return side_; }
  
        // ------------------------------------------ subtract min from all rows cols
        // This prepares the data for the algorithm
        void subtract_min_from_all_rows_cols()
        {
           auto min_val_in_row = [&](uint32 r) -> T
           {
              auto min_val = C(r, 0);
              for (auto c = 1u; c < side_; ++c)
                 if (C(r, c) < min_val) min_val = C(r, c);
              return min_val;
           };
  
           auto min_val_in_col = [&](uint32 c) -> T
           {
              auto min_val = C(0, c);
              for (auto r = 1u; r < side_; ++r)
                 if (C(r, c) < min_val) min_val = C(r, c);
              return min_val;
           };
  
           // Minimize each row
           for (auto r = 0u; r < side_; ++r)
           {
              const auto min_val = min_val_in_row(r);
              for (auto c = 0u; c < side_; ++c)
                 C(r, c) -= min_val;
           }
  
           // Minimize each col
           for (auto c = 0u; c < side_; ++c)
           {
              const auto min_val = min_val_in_col(c);
              for (auto r = 0u; r < side_; ++r)
                 C(r, c) -= min_val;
           }
        }
  
        // ------------------------------------------------------------------- Step 1
        // Iterate over each element...
        // If it's 0, and there's no other zero in row/col, then STAR
        int step1() noexcept
        {
           std::vector<bool> r_mask(side(), false);
           std::vector<bool> c_mask(side(), false);
           for (auto r = 0u; r < side(); ++r)
           {
              if (r_mask[r]) continue;
              for (auto c = 0u; c < side(); ++c)
              {
                 if (r_mask[r] || c_mask[c]) continue;
                 if (C(r, c) == T(0)) {
                    M(r, c)   = STAR;
                    r_mask[r] = true;
                    c_mask[c] = true;
                 }
              }
           }
           return 2;
        }
  
        // ------------------------------------------------------------------- Step 2
        // Cover each column containing a STAR
        int step2() noexcept
        {
           auto counter = 0u;
  
           for (auto c = 0u; c < side(); ++c) assert(!is_col_covered(c));
  
           for (auto r = 0u; r < side(); ++r) {
              for (auto c = 0u; c < side(); ++c) {
                 if (is_col_covered(c)) continue;
                 if (M(r, c) == STAR) {
                    cover_col(c);
                    counter++;
                 }
              }
           }
  
           // A complete matching
           if (counter >= side()) return 0;
  
           return 3;
        }
  
        // ------------------------------------------------------------------- Step 3
        // Find a uncovered zero and PRIME it.
        // Eventually get to a state where the PRIMEd row contains no STAR zeros
        std::tuple<int, uint32, uint32> step3() noexcept
        {
           auto find_uncovered_row_col = [&](uint32& r, uint32& c) -> bool {
              for (r = 0; r < side_; ++r)
                 if (!is_row_covered(r))
                    for (c = 0; c < side_; ++c)
                       if (!is_col_covered(c))
                          if (C(r, c) == T(0)) return true;
              return false;
           };
  
           // Find an uncovered zero, and mark it PRIME
           uint32 saved_row = 0, saved_col = 0;
           if (find_uncovered_row_col(saved_row, saved_col))
              M(saved_row, saved_col) = PRIME;
           else
              return std::tuple<int, uint32, uint32>{5, saved_row, saved_col}; // all zeros covered
  
           // If there's a STAR in the PRIMEd row, then:
           for (auto c = 0u; c < side(); ++c) {
              if (M(saved_row, c) == STAR) {
                 cover_row(saved_row);             // cover that row
                 uncover_col(c);                   // uncover the column
                 return std::tuple<int, uint32, uint32>{3, saved_row, saved_col}; // and repeat this step
              }
           }
  
           // There's no STAR in the PRIMEd row, onto "augmenting path"
           return std::tuple<int, uint32, uint32>{4, saved_row, saved_col};
        }
  
        // ------------------------------------------------------------------- Step 4
        // Augmenting path algorithm
        int step4(const uint32 saved_row, const uint32 saved_col) noexcept
        {
           auto find_star_in_col = [&](const uint32 c) -> int {
              for (auto r = 0u; r < side(); ++r)
                 if (M(r, c) == STAR) return r;
              return -1; // row not found
           };
  
           auto find_prime_in_row = [&](const uint32 r) -> int {
              for (auto c = 0u; c < side(); ++c)
                 if (M(r, c) == PRIME) return c;
              assert(false); // we should ALWAYS find this column
              return -1;     // col not found
           };
  
           auto make_path = [&](const edge e0) {
              std::vector<edge> seq;
              seq.reserve(side());
              seq.push_back(e0);
              int r = -1, c = -1;
              while (true) {
                 c = seq.back().second;
                 r = find_star_in_col(c); // STARed zero in column of PRIMEd back()
                 if (r >= 0)
                    seq.push_back({r, c}); // Push a STAR edge
                 else                      // If it doesn't exist, then the path is done
                    break;
                 c = find_prime_in_row(r);
                 seq.push_back({r, c}); // Push a PRIME edge
              }
              return seq;
           };
  
           auto augment_path = [&](const std::vector<edge>& seq) {
              // For all edges in sequence:
              //    1. Erase all STARs
              //    2. And convert all PRIMEs to STARs
              for (const auto& e : seq) {
                 if (M(e.first, e.second) == STAR)
                    M(e.first, e.second) = NONE;
                 else if (M(e.first, e.second) == PRIME)
                    M(e.first, e.second) = STAR;
              }
           };
  
           auto erase_primes = [&]() {
              for (auto r = 0u; r < side(); ++r)
                 for (auto c = 0u; c < side(); ++c)
                    if (M(r, c) == PRIME) M(r, c) = NONE;
           };
  
           auto clear_covers = [&]() {
              std::fill(begin(row_mask), end(row_mask), false);
              std::fill(begin(col_mask), end(col_mask), false);
           };
  
           const edge e0{saved_row, saved_col}; // Uncovered primed zero from step3
           auto seq = make_path(e0);
           augment_path(seq);
           erase_primes();
           clear_covers();
  
           return 2;
        }
  
        // ------------------------------------------------------------------- Step 5
        // Find the smallest uncovered value, and:
        //   1. Add it to every covered row
        //   2. Subtract it from every uncovered col
        int step5() noexcept
        {
           auto find_min_uncovered_value = [&]() {
              auto minval = std::numeric_limits<T>::max();
              for (auto r = 0u; r < side(); ++r) {
                 if (is_row_covered(r)) continue;
                 for (auto c = 0u; c < side(); c++) {
                    if (is_col_covered(c)) continue;
                    if (C(r, c) < minval) minval = C(r, c);
                 }
              }
              return minval;
           };
           
           const auto minval = find_min_uncovered_value();
  
           for (auto r = 0u; r < side(); ++r)
           {
              for (auto c = 0u; c < side(); c++)
              {
                 if ( is_row_covered(r)) C(r, c) += minval; // (1) add minval
                 if (!is_col_covered(c)) C(r, c) -= minval; // (2) subtract minval
              }
           }
  
           return 3;
        }
  
        std::vector<edge> solve() noexcept
        {
            // The Munkres Algorithm is described as a state machine
            int step = 1;
            uint32 saved_row = 0, saved_col = 0;
            while (step)
            {
                switch (step)
                {
                case 1:
                    step = step1(); // => [2]
                    break;
                case 2:
                    step = step2(); // => [0, 3]
                    break;
                case 3:
                    std::tie(step, saved_row, saved_col) = step3(); // => [3, 4, 5]
                    break;
                case 4:
                    step = step4(saved_row, saved_col); // => [2]
                    break;
                case 5:
                    step = step5(); // => [3]
                    break;
                }
            }
   
            // Collate the results
            std::vector<edge> out;
            out.reserve(side());
            for (auto r = 0u; r < original_rows(); ++r)
               for (auto c = 0u; c < original_cols(); ++c)
                  if (M(r, c) == STAR)
                     out.push_back({r, c});
            return out;
        }
 
 
       
    }; // End of Struct MunkresData
} // End of Namespace detail

//
// @param n_lhs_verts Number of left-hand-side vertices (in bipartite graph)
// @param n_rhs_verts Number of right-hand-side verices (ibid)
// @param cost Cost between vertices 'l' and 'r'. Use of function to abstract
//             away storage details of input graph.
// @see example.cpp
//
template<typename T>
std::vector<std::pair<uint32, uint32>> inline munkres_algorithm(
    const uint32 n_lhs_verts,
    const uint32 n_rhs_verts,
    std::function<T(uint32 l, uint32 r)> cost) noexcept
{
   detail::MunkresData<T> m_dat{n_lhs_verts, n_rhs_verts, cost};
   return m_dat.solve();
}







//https://zhuanlan.zhihu.com/p/267074414
class GF_KuhnMunkres
{
public:
    int KM()
    {
        memset(match, -1, sizeof match);    // 初始每个男生都没有匹配的女生
        memset(ex_boy, 0, sizeof ex_boy);   // 初始每个男生的期望值为0

        // 每个女生的初始期望值是与她相连的男生最大的好感度
        for (int i = 0; i < N; ++i) {
            ex_girl[i] = love[i][0];
            for (int j = 1; j < N; ++j) {
                ex_girl[i] = SYSmax(ex_girl[i], love[i][j]);
            }
        }

        // 尝试为每一个女生解决归宿问题
        for (int i = 0; i < N; ++i) {

            fill(slack, slack + N, INF);    // 因为要取最小值 初始化为无穷大

            while (1) {
                // 为每个女生解决归宿问题的方法是 ：如果找不到就降低期望值，直到找到为止

                // 记录每轮匹配中男生女生是否被尝试匹配过
                memset(vis_girl, false, sizeof vis_girl);
                memset(vis_boy, false, sizeof vis_boy);

                if (dfs(i)) break;  // 找到归宿 退出

                // 如果不能找到 就降低期望值
                // 最小可降低的期望值
                int d = INF;
                for (int j = 0; j < N; ++j)
                    if (!vis_boy[j]) d = SYSmin(d, slack[j]);

                for (int j = 0; j < N; ++j) {
                    // 所有访问过的女生降低期望值
                    if (vis_girl[j]) ex_girl[j] -= d;

                    // 所有访问过的男生增加期望值
                    if (vis_boy[j]) ex_boy[j] += d;
                    // 没有访问过的boy 因为girl们的期望值降低，距离得到女生倾心又进了一步！
                    else slack[j] -= d;
                }
            }
        }

        // 匹配完成 求出所有配对的好感度的和
        int res = 0;
        for (int i = 0; i < N; ++i)
            res += love[match[i]][i];

        return res;
    }



private:
    const int MAXN = 305;
    const int INF = 0x3f3f3f3f;


    int love[MAXN][MAXN];   // 记录每个妹子和每个男生的好感度
    int ex_girl[MAXN];      // 每个妹子的期望值
    int ex_boy[MAXN];       // 每个男生的期望值
    bool vis_girl[MAXN];    // 记录每一轮匹配匹配过的女生
    bool vis_boy[MAXN];     // 记录每一轮匹配匹配过的男生
    int match[MAXN];        // 记录每个男生匹配到的妹子 如果没有则为-1
    int slack[MAXN];        // 记录每个汉子如果能被妹子倾心最少还需要多少期望值

    int N;


private:
    bool dfs(int girl)
    {
        vis_girl[girl] = true;

        for (int boy = 0; boy < N; ++boy) {

            if (vis_boy[boy]) continue; // 每一轮匹配 每个男生只尝试一次

            int gap = ex_girl[girl] + ex_boy[boy] - love[girl][boy];

            if (gap == 0) {  // 如果符合要求
                vis_boy[boy] = true;
                if (match[boy] == -1 || dfs(match[boy])) {    // 找到一个没有匹配的男生 或者该男生的妹子可以找到其他人
                    match[boy] = girl;
                    return true;
                }
            }
            else {
                slack[boy] = SYSmin(slack[boy], gap);  // slack 可以理解为该男生要得到女生的倾心 还需多少期望值 取最小值 备胎的样子【捂脸
            }
        }

        return false;
    }


};

namespace GFE_KuhnMunkres {


    template<typename FLOAT_T>
    static void
        km(
            GA_Detail* const geo,
            const GA_Detail* const refGeo,
            const GA_RWHandleT<GA_Offset>& kmMatchedPoint_h,
            const GA_ROHandleT<FLOAT_T>& weight
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(refGeo);


    }




template<typename VECTOR_T>
static void
km(
    GA_Detail* const geo,
    const GA_Detail* const refGeo,
    const GA_RWHandleT<GA_Offset>& kmMatchedPoint_h,
    const GA_ROHandleT<VECTOR_T>& pos_h,
    const GA_ROHandleT<VECTOR_T>& posRef_h
)
{
    UT_ASSERT_P(geo);
    UT_ASSERT_P(refGeo);

    GA_Index indexSize    = pos_h   .getAttribute()->getIndexMap().indexSize();
    GA_Index indexSizeRef = posRef_h.getAttribute()->getIndexMap().indexSize();
    
    std::vector<indexSizeRef>
    const GA_ROHandleT<FLOAT_T>& weight_h
    km<UT_Vector3T<fpreal16>>(geo, refGeo, kmMatchedPointAttribPtr, posAttribPtr, posRefAttribPtr);
}



static GA_Attribute*
    km(
        GA_Detail* const geo,
        const GA_Detail* const refGeo,
        const UT_StringHolder& kmMatchedPointName = "kmMatchedPoint",
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringHolder& posAttribName = "P",
        const UT_StringHolder& refPosAttribName = "P"
    ) {
    UT_ASSERT_P(geo);
    UT_ASSERT_P(refGeo);

    if (!kmMatchedPointName.isstring() || kmMatchedPointName.isEmpty())
        return nullptr;


    const GA_Storage finalStorage = storage == GA_STORE_INVALID ? GFE_Type::getPreferredStorageI(geo) : storage;

    GA_Attribute* kmMatchedPointAttribPtr = geo->findPointAttribute(kmMatchedPointName);
    if (kmMatchedPointAttribPtr && kmMatchedPointAttribPtr->getStorageClass() != GA_STORECLASS_INT)
    {
        geo->getAttributes().destroyAttribute(kmMatchedPointAttribPtr);
        kmMatchedPointAttribPtr = nullptr;
    }

    if (!kmMatchedPointAttribPtr)
        kmMatchedPointAttribPtr = geo->getAttributes().createTupleAttribute(GA_ATTRIB_POINT, kmMatchedPointName, finalStorage, 1, GA_Defaults(-1));

    const GA_Attribute* posAttribPtr = geo->findPointAttribute(kmMatchedPointName);
    if (!posAttribPtr)
        posAttribPtr = geo->getP();

    const GA_Attribute* posRefAttribPtr = refGeo->findPointAttribute(kmMatchedPointName);
    if (!posRefAttribPtr)
        posRefAttribPtr = refGeo->getP();

    switch (posAttribPtr->getAIFTuple()->getStorage(posAttribPtr))
    {
    case GA_STORE_REAL16:
        km<UT_Vector3T<fpreal16>>(geo, refGeo, kmMatchedPointAttribPtr, posAttribPtr, posRefAttribPtr);
        break;
    case GA_STORE_REAL32:
        km<UT_Vector3T<fpreal32>>(geo, refGeo, kmMatchedPointAttribPtr, posAttribPtr, posRefAttribPtr);
        break;
    case GA_STORE_REAL64:
        km<UT_Vector3T<fpreal64>>(geo, refGeo, kmMatchedPointAttribPtr, posAttribPtr, posRefAttribPtr);
        break;
    default:
        break;
    }

    return kmMatchedPointAttribPtr;
}





static GA_Attribute*
    km(
        const SOP_NodeVerb::CookParms& cookparms,
        GA_Detail* const geo,
        const GA_Detail* const refGeo,
        const UT_StringHolder& kmMatchedPointName = "kmMatchedPoint",
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringHolder& posAttribName = "P",
        const UT_StringHolder& refPosAttribName = "P"
    ) {
    UT_ASSERT_P(geo);
    UT_ASSERT_P(refGeo);

    GA_Attribute* kmMatchedPointAttribPtr = km(geo, refGeo, kmMatchedPointName, storage, posAttribName, refPosAttribName);

    kmMatchedPointAttribPtr->bumpDataId();
    return kmMatchedPointAttribPtr;
}







} // End of namespace GFE_KuhnMunkres

#endif
