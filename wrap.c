/* {{{

    This file is part of libtraceproc - a library for tracing Pro*C/OCI calls

    Copyright (C) 2013 Georg Sauthoff <mail@georg.so>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

}}} */


// for dlfcn.h
#if defined(__linux__)
#define _GNU_SOURCE
#endif

#include "foreign.h"
#include "intercept.h"

#include "ret_check.h"
#include "timespec.h"

#include "prettyprint.h"
#include "trace.h"

#include "callback.h"
#include "stats.h"

#include "ocitrace.h"
#include "trap.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <time.h>

#include <sqlcpr.h>



static const char *statement_type_str[] =
{
  0, // 0
  "DROP TBL|SYN|ALTER", // 1
  "DELETE", // 2
  "INSERT", // 3
  "SELECT", // 4
  "UPDATE", // 5
  0, // 6
  0, // 7
  0, // 8
  "OPEN cursor", // 9
  0, // 10
  0, // 11
  0, // 12
  "FETCH", // 13 // cursor
  0, // 14
  "CLOSE", // 15 // cursor
  0, // 16
  "PREPARE", // 17 // oracle dynamic SQL
  0, // 18
  0, // 19
  0, // 20
  "EXECUTE", // 21 // oracle dynamic SQL
  0, // 22
  0, // 23
  "EXECUTE IMMEDIATE", // 24 // oracle dynamic SQL
  0, // 25
  0, // 26
  "CONNECT", // 27
  0, // 28
  "COMMIT", // 29
  "COMMIT RELEASE", // 30
  "ROLLBACK", // 31
  "ROLLBACK RELEASE", // 32
  0, // 33
  0, // 34
  0, // 35
  0, // 36
  0, // 37
  0, // 38
  0, // 39
  0, // 40
  0, // 41
  0, // 42
  0, // 43
  "CREATE TABLE|SYNONYM", // 44
  "OPEN cursor dyn", // 45 // cursor, oracle dynamic SQL
  0, // 46
  0, // 47
  0, // 48
  0, // 49
  0, // 50
  0, // 51
  0, // 52
  0, // 53
  0, // 54
  0, // 55
  0, // 56
  0, // 57
  0, // 58
  0, // 59
  0, // 60
  0, // 61
  0, // 62
  0, // 63
  0, // 64
  0, // 65
  0, // 66
  0, // 67
  0, // 68
  0, // 69
  0, // 70
  0, // 71
  0, // 72
  0, // 73
  0, // 74
  0, // 75
  0, // 76
  0, // 77
  0, // 78
  0, // 79
  0, // 80
  0, // 81
  0, // 82
  0, // 83
  0, // 84
  0, // 85
  0, // 86
  0, // 87
  0, // 88
  0, // 89
  0, // 90
  0, // 91
  0, // 92
  0, // 93
  0, // 94
  0, // 95
  0, // 96
  0, // 97
  0, // 98
  0, // 99
  0, // 100
  0, // 101
  0, // 102
  0, // 103
  0, // 104
  0, // 105
  0, // 106
  0, // 107
  0, // 108
  0, // 109
  0, // 110
  0, // 111
  0, // 112
  0, // 113
  0, // 114
  0, // 115
  0, // 116
  0, // 117
  0, // 118
  0, // 119
  0, // 120
  0, // 121
  "CALL", // 122 // call stored procedure/function
  0, // 123
  0, // 124
  0, // 125
  0, // 126
  0, // 127
  0, // 128
  0, // 129
  0, // 130
  0, // 131
  0, // 132
  0, // 133
  0, // 134
  0, // 135
  0, // 136
  0, // 137
  0, // 138
  0, // 139
  0, // 140
  0, // 141
  0, // 142
  0, // 143
  0, // 144
  0, // 145
  0, // 146
  0, // 147
  0, // 148
  0, // 149
  0, // 150
  0, // 151
  0, // 152
  0, // 153
  0, // 154
  0, // 155
  0, // 156
  0, // 157
  0, // 158
  0, // 159
  0, // 160
  0, // 161
  0, // 162
  0, // 163
  0, // 164
  0, // 165
  0, // 166
  0, // 167
  0, // 168
  0, // 169
  0, // 170
  0, // 171
  0, // 172
  0, // 173
  0, // 174
  0, // 175
  0, // 176
  0, // 177
  0, // 178
  0, // 179
  0, // 180
  0, // 181
  0, // 182
  0, // 183
  0, // 184
  0, // 185
  0, // 186
  0, // 187
  0, // 188
  0, // 189
  0, // 190
  0, // 191
  0, // 192
  0, // 193
  0, // 194
  0, // 195
  0, // 196
  0, // 197
  0, // 198
  0, // 199
  0, // 200
  0, // 201
  0, // 202
  0, // 203
  0, // 204
  0, // 205
  0, // 206
  0, // 207
  0, // 208
  0, // 209
  0, // 210
  0, // 211
  0, // 212
  0, // 213
  0, // 214
  0, // 215
  0, // 216
  0, // 217
  0, // 218
  0, // 219
  0, // 220
  0, // 221
  0, // 222
  0, // 223
  0, // 224
  0, // 225
  0, // 226
  0, // 227
  0, // 228
  0, // 229
  0, // 230
  0, // 231
  0, // 232
  0, // 233
  0, // 234
  0, // 235
  0, // 236
  0, // 237
  0, // 238
  0, // 239
  0, // 240
  0, // 241
  0, // 242
  0, // 243
  0, // 244
  0, // 245
  0, // 246
  0, // 247
  0, // 248
  0, // 249
  0, // 250
  0, // 251
  0, // 252
  0, // 253
  0, // 254
  0, // 255
  0, // 256
  0, // 257
  0, // 258
  0, // 259
  0, // 260
  0, // 261
  0, // 262
  0, // 263
  0, // 264
  0, // 265
  0, // 266
  0, // 267
  0, // 268
  0, // 269
  0, // 270
  0, // 271
  0, // 272
  0, // 273
  0, // 274
  0, // 275
  0, // 276
  0, // 277
  0, // 278
  0, // 279
  0, // 280
  0, // 281
  0, // 282
  0, // 283
  0, // 284
  0, // 285
  0, // 286
  0, // 287
  0, // 288
  0, // 289
  0, // 290
  0, // 291
  0, // 292
  0, // 293
  0, // 294
  0, // 295
  0, // 296
  0, // 297
  0, // 298
  0, // 299
  0, // 300
  0, // 301
  0, // 302
  0, // 303
  0, // 304
  0, // 305
  0, // 306
  0, // 307
  0, // 308
  0, // 309
  0, // 310
  0, // 311
  0, // 312
  0, // 313
  0, // 314
  0, // 315
  0, // 316
  0, // 317
  0, // 318
  0, // 319
  0, // 320
  0, // 321
  0, // 322
  0, // 323
  0, // 324
  0, // 325
  0, // 326
  0, // 327
  0, // 328
  0, // 329
  0, // 330
  0, // 331
  0, // 332
  0, // 333
  0, // 334
  0, // 335
  0, // 336
  0, // 337
  0, // 338
  0, // 339
  0, // 340
  0, // 341
  0, // 342
  0, // 343
  0, // 344
  0, // 345
  0, // 346
  0, // 347
  0, // 348
  0, // 349
  0, // 350
  0, // 351
  0, // 352
  0, // 353
  0, // 354
  0, // 355
  0, // 356
  0, // 357
  0, // 358
  0, // 359
  0, // 360
  0, // 361
  0, // 362
  0, // 363
  0, // 364
  0, // 365
  0, // 366
  0, // 367
  0, // 368
  0, // 369
  0, // 370
  0, // 371
  0, // 372
  0, // 373
  0, // 374
  0, // 375
  0, // 376
  0, // 377
  0, // 378
  0, // 379
  0, // 380
  0, // 381
  0, // 382
  0, // 383
  0, // 384
  0, // 385
  0, // 386
  0, // 387
  0, // 388
  0, // 389
  0, // 390
  0, // 391
  0, // 392
  0, // 393
  0, // 394
  0, // 395
  0, // 396
  0, // 397
  0, // 398
  0, // 399
  0, // 400
  0, // 401
  0, // 402
  0, // 403
  0, // 404
  0, // 405
  0, // 406
  0, // 407
  0, // 408
  0, // 409
  0, // 410
  0, // 411
  0, // 412
  0, // 413
  0, // 414
  0, // 415
  0, // 416
  0, // 417
  0, // 418
  0, // 419
  0, // 420
  0, // 421
  0, // 422
  0, // 423
  0, // 424
  0, // 425
  0, // 426
  0, // 427
  0, // 428
  0, // 429
  0, // 430
  0, // 431
  0, // 432
  0, // 433
  0, // 434
  0, // 435
  0, // 436
  0, // 437
  0, // 438
  0, // 439
  0, // 440
  0, // 441
  0, // 442
  0, // 443
  0, // 444
  0, // 445
  0, // 446
  0, // 447
  0, // 448
  0, // 449
  0, // 450
  0, // 451
  0, // 452
  0, // 453
  0, // 454
  0, // 455
  0, // 456
  0, // 457
  0, // 458
  0, // 459
  0, // 460
  0, // 461
  0, // 462
  0, // 463
  0, // 464
  0, // 465
  0, // 466
  0, // 467
  0, // 468
  0, // 469
  0, // 470
  0, // 471
  0, // 472
  0, // 473
  0, // 474
  0, // 475
  0, // 476
  0, // 477
  0, // 478
  0, // 479
  0, // 480
  0, // 481
  0, // 482
  0, // 483
  0, // 484
  0, // 485
  0, // 486
  0, // 487
  0, // 488
  0, // 489
  0, // 490
  0, // 491
  0, // 492
  0, // 493
  0, // 494
  0, // 495
  0, // 496
  0, // 497
  0, // 498
  0, // 499
  0, // 500
  0, // 501
  0, // 502
  0, // 503
  0, // 504
  0, // 505
  0, // 506
  0, // 507
  0, // 508
  0, // 509
  0, // 510
  0, // 511
  0, // 512
  0, // 513
  0, // 514
  0, // 515
  0, // 516
  0, // 517
  0, // 518
  0, // 519
  0, // 520
  0, // 521
  0, // 522
  0, // 523
  0, // 524
  0, // 525
  0, // 526
  0, // 527
  0, // 528
  0, // 529
  0, // 530
  0, // 531
  0, // 532
  0, // 533
  0, // 534
  0, // 535
  0, // 536
  0, // 537
  0, // 538
  0, // 539
  0, // 540
  0, // 541
  0, // 542
  0, // 543
  0, // 544
  0, // 545
  0, // 546
  0, // 547
  0, // 548
  0, // 549
  0, // 550
  0, // 551
  0, // 552
  0, // 553
  0, // 554
  0, // 555
  0, // 556
  0, // 557
  0, // 558
  0, // 559
  0, // 560
  0, // 561
  0, // 562
  0, // 563
  0, // 564
  0, // 565
  0, // 566
  0, // 567
  0, // 568
  0, // 569
  0, // 570
  0, // 571
  0, // 572
  0, // 573
  0, // 574
  0, // 575
  0, // 576
  0, // 577
  0, // 578
  0, // 579
  0, // 580
  0, // 581
  0, // 582
  0, // 583
  0, // 584
  0, // 585
  0, // 586
  0, // 587
  0, // 588
  0, // 589
  0, // 590
  0, // 591
  0, // 592
  0, // 593
  0, // 594
  0, // 595
  0, // 596
  0, // 597
  0, // 598
  0, // 599
  0, // 600
  0, // 601
  0, // 602
  0, // 603
  0, // 604
  0, // 605
  0, // 606
  0, // 607
  0, // 608
  0, // 609
  0, // 610
  0, // 611
  0, // 612
  0, // 613
  0, // 614
  0, // 615
  0, // 616
  0, // 617
  0, // 618
  0, // 619
  0, // 620
  0, // 621
  0, // 622
  0, // 623
  0, // 624
  0, // 625
  0, // 626
  0, // 627
  0, // 628
  0, // 629
  0, // 630
  0, // 631
  0, // 632
  0, // 633
  0, // 634
  0, // 635
  0, // 636
  0, // 637
  0, // 638
  0, // 639
  0, // 640
  0, // 641
  0, // 642
  0, // 643
  0, // 644
  0, // 645
  0, // 646
  0, // 647
  0, // 648
  0, // 649
  0, // 650
  0, // 651
  0, // 652
  0, // 653
  0, // 654
  0, // 655
  0, // 656
  0, // 657
  0, // 658
  0, // 659
  0, // 660
  0, // 661
  0, // 662
  0, // 663
  0, // 664
  0, // 665
  0, // 666
  0, // 667
  0, // 668
  0, // 669
  0, // 670
  0, // 671
  0, // 672
  0, // 673
  0, // 674
  0, // 675
  0, // 676
  0, // 677
  0, // 678
  0, // 679
  0, // 680
  0, // 681
  0, // 682
  0, // 683
  0, // 684
  0, // 685
  0, // 686
  0, // 687
  0, // 688
  0, // 689
  0, // 690
  0, // 691
  0, // 692
  0, // 693
  0, // 694
  0, // 695
  0, // 696
  0, // 697
  0, // 698
  0, // 699
  0, // 700
  0, // 701
  0, // 702
  0, // 703
  0, // 704
  0, // 705
  0, // 706
  0, // 707
  0, // 708
  0, // 709
  0, // 710
  0, // 711
  0, // 712
  0, // 713
  0, // 714
  0, // 715
  0, // 716
  0, // 717
  0, // 718
  0, // 719
  0, // 720
  0, // 721
  0, // 722
  0, // 723
  0, // 724
  0, // 725
  0, // 726
  0, // 727
  0, // 728
  0, // 729
  0, // 730
  0, // 731
  0, // 732
  0, // 733
  0, // 734
  0, // 735
  0, // 736
  0, // 737
  0, // 738
  0, // 739
  0, // 740
  0, // 741
  0, // 742
  0, // 743
  0, // 744
  0, // 745
  0, // 746
  0, // 747
  0, // 748
  0, // 749
  0, // 750
  0, // 751
  0, // 752
  0, // 753
  0, // 754
  0, // 755
  0, // 756
  0, // 757
  0, // 758
  0, // 759
  0, // 760
  0, // 761
  0, // 762
  0, // 763
  0, // 764
  0, // 765
  0, // 766
  0, // 767
  0, // 768
  0, // 769
  0, // 770
  0, // 771
  0, // 772
  0, // 773
  0, // 774
  0, // 775
  0, // 776
  0, // 777
  0, // 778
  0, // 779
  0, // 780
  0, // 781
  0, // 782
  0, // 783
  0, // 784
  0, // 785
  0, // 786
  0, // 787
  0, // 788
  0, // 789
  0, // 790
  0, // 791
  0, // 792
  0, // 793
  0, // 794
  0, // 795
  0, // 796
  0, // 797
  0, // 798
  0, // 799
  0, // 800
  0, // 801
  0, // 802
  0, // 803
  0, // 804
  0, // 805
  0, // 806
  0, // 807
  0, // 808
  0, // 809
  0, // 810
  0, // 811
  0, // 812
  0, // 813
  0, // 814
  0, // 815
  0, // 816
  0, // 817
  0, // 818
  0, // 819
  0, // 820
  0, // 821
  0, // 822
  0, // 823
  0, // 824
  0, // 825
  0, // 826
  0, // 827
  0, // 828
  0, // 829
  0, // 830
  0, // 831
  0, // 832
  0, // 833
  0, // 834
  0, // 835
  0, // 836
  0, // 837
  0, // 838
  0, // 839
  0, // 840
  0, // 841
  0, // 842
  0, // 843
  0, // 844
  0, // 845
  0, // 846
  0, // 847
  0, // 848
  0, // 849
  0, // 850
  0, // 851
  0, // 852
  0, // 853
  0, // 854
  0, // 855
  0, // 856
  0, // 857
  0, // 858
  0, // 859
  0, // 860
  0, // 861
  0, // 862
  0, // 863
  0, // 864
  0, // 865
  0, // 866
  0, // 867
  0, // 868
  0, // 869
  0, // 870
  0, // 871
  0, // 872
  0, // 873
  0, // 874
  0, // 875
  0, // 876
  0, // 877
  0, // 878
  0, // 879
  0, // 880
  0, // 881
  0, // 882
  0, // 883
  0, // 884
  0, // 885
  0, // 886
  0, // 887
  0, // 888
  0, // 889
  0, // 890
  0, // 891
  0, // 892
  0, // 893
  0, // 894
  0, // 895
  0, // 896
  0, // 897
  0, // 898
  0, // 899
  0, // 900
  0, // 901
  0, // 902
  0, // 903
  0, // 904
  0, // 905
  0, // 906
  0, // 907
  0, // 908
  0, // 909
  0, // 910
  0, // 911
  0, // 912
  0, // 913
  0, // 914
  0, // 915
  0, // 916
  0, // 917
  0, // 918
  0, // 919
  0, // 920
  0, // 921
  0, // 922
  0, // 923
  0, // 924
  0, // 925
  0, // 926
  0, // 927
  0, // 928
  0, // 929
  0, // 930
  0, // 931
  0, // 932
  0, // 933
  0, // 934
  0, // 935
  0, // 936
  0, // 937
  0, // 938
  0, // 939
  0, // 940
  0, // 941
  0, // 942
  0, // 943
  0, // 944
  0, // 945
  0, // 946
  0, // 947
  0, // 948
  0, // 949
  0, // 950
  0, // 951
  0, // 952
  0, // 953
  0, // 954
  0, // 955
  0, // 956
  0, // 957
  0, // 958
  0, // 959
  0, // 960
  0, // 961
  0, // 962
  0, // 963
  0, // 964
  0, // 965
  0, // 966
  0, // 967
  0, // 968
  0, // 969
  0, // 970
  0, // 971
  0, // 972
  0, // 973
  0, // 974
  0, // 975
  0, // 976
  0, // 977
  0, // 978
  0, // 979
  0, // 980
  0, // 981
  0, // 982
  0, // 983
  0, // 984
  0, // 985
  0, // 986
  0, // 987
  0, // 988
  0, // 989
  0, // 990
  0, // 991
  0, // 992
  0, // 993
  0, // 994
  0, // 995
  0, // 996
  0, // 997
  0, // 998
  0, // 999
  0, // 1000
  0, // 1001
  0, // 1002
  0, // 1003
  0, // 1004
  0, // 1005
  0, // 1006
  0, // 1007
  0, // 1008
  0, // 1009
  0, // 1010
  0, // 1011
  0, // 1012
  0, // 1013
  0, // 1014
  0, // 1015
  0, // 1016
  0, // 1017
  0, // 1018
  0, // 1019
  0, // 1020
  0, // 1021
  0, // 1022
  0, // 1023
  0, // 1024
  0, // 1025
  0, // 1026
  0, // 1027
  0, // 1028
  0, // 1029
  0, // 1030
  0, // 1031
  0, // 1032
  0, // 1033
  0, // 1034
  0, // 1035
  0, // 1036
  0, // 1037
  0, // 1038
  0, // 1039
  0, // 1040
  0, // 1041
  0, // 1042
  0, // 1043
  0, // 1044
  0, // 1045
  0, // 1046
  0, // 1047
  0, // 1048
  0, // 1049
  0, // 1050
  "CONNECT AT", // 1051 // CONNECT ... AT
};
#define WRAP_STATEMENT_TYPE_SIZE 1052 
static size_t statement_type_str_size = WRAP_STATEMENT_TYPE_SIZE;
static const char *fn_to_str(unsigned i)
{
  if (i>=statement_type_str_size)
    return "0: OUT_OF_BOUNDS";
  const char *s = statement_type_str[i];
  if (s)
    return s;
  return "0: UNK";
}


struct Default_SQL {
  Statement_Type stmt;
  const char *text;
};
typedef struct Default_SQL Default_SQL;

static Default_SQL default_sql[] = {
  { CONNECT, "CONNECT :username IDENTIFIED BY :password "
                "USING :dbspec" },
  { CONNECT_AT, "CONNECT :username IDENTIFIED BY :password "
                   "AT :dbname USING :dbspec" },
  { COMMIT, "COMMIT" },
  { COMMIT_RELEASE, "COMMIT WORK RELEASE" },
  { ROLLBACK, "ROLLBACK" },
  { ROLLBACK_RELEASE, "ROLLBACK WORK RELEASE" },
  { 0, 0}
};

static const char *default_sql_text(Statement_Type code)
{
  Default_SQL *d = default_sql;
  for (;d->stmt;++d)
    if (d->stmt == code)
      return d->text;
  return 0;
}

struct type_info {
  type_no no;
  const char *str; 
};
typedef struct type_info type_info;
static const type_info sql_type_info[] = {
  { INT , "INT" },
  { DOUBLE, "DOUBLE" },
  { STRING0, "STRING0" },
  { VARCHAR, "VARCHAR" },
  { VOID, "VOID?" },
  { UNSIGNED, "UNSIGNED" },
  { STRING, "STRING" },
  { CHAR, "CHAR" },
  { 0, 0}
};
static const char *type_to_str(short t)
{
  const type_info *i = sql_type_info;
  for (; i->no; ++i)
  {
    if (i->no == t)
      return i->str;
  }
  return "0: TYPE_UNK";
}

typedef struct Fetch_Info Fetch_Info;
static const Fetch_Info fetch_infos[] = {
  { FETCH_CURRENT, "FETCH_CURRENT"} ,
  { FETCH_NEXT, "FETCH_NEXT"} ,
  { FETCH_FIRST, "FETCH_FIRST"} ,
  { FETCH_LAST, "FETCH_LAST"} ,
  { FETCH_PRIOR, "FETCH_PRIOR"} ,
  { FETCH_ABSOLUTE, "FETCH_ABSOLUTE"} ,
  { FETCH_RELATIVE, "FETCH_RELATIVE"} ,
  {0, 0}
};
static const char *fetch_to_str(unsigned t)
{
  const Fetch_Info *i = fetch_infos;
  for (; i->type; ++i)
  {
    if (i->type == t)
      return i->str;
  }
  return "0: FETCH_UNK";
}


enum Func {
  FN_SQLCXT  = 0,
  FN_SQLORAT = 1,
  FN_SIZE = 2
};
typedef enum Func Func;
static const char *func_str[] = {
  "sqlcxt",
  "sqlorat"
};

static Stats stats = {0};

static int pp_stats_sql()
{
  tprintf("%20s | %10s | %14s | %4s\n",
      "STATEMENT", "COUNT", "TIME (s)", "%");
  tprintf("---------------------+------------+----------------+-----\n");
  for (unsigned i = 0; i<statement_type_str_size; ++i) {
    if (!stats.counts[i])
      continue;
    char i_str[10] = {0};
    snprintf(i_str, 10, "%u", i);
    char t[15] = {0};
    timespec_pp_intv(stats.times + i, t, 15);

    tprintf("%20s | %10zu | %14s | %.2f\n",
        statement_type_str[i] ? statement_type_str[i] :i_str,
        stats.counts[i],
        t,
        timespec_cent(stats.times + i, &stats.time_sum)
        );
  }
  tprintf("=====================+============+================+=====\n");
  char t[15] = {0};
  timespec_pp_intv(&stats.time_sum, t, 15);
  tprintf("%20s | %10zu | %14s | %4s\n",
      "SUM",stats.count_sum, t, "1.00");
  return 0;
}


enum Binary_Option {
  OPT_INTERCEPT = 0,
  OPT_STATS,
  OPT_SQL,
  OPT_GORY,
  OPT_BEFORE,
  OPT_AFTER,
  OPT_HELP,
  OPT_ORAT,
  OPT_FRAME,
  OPT_TIME,
  OPT_OCI,
  OPT_LEAK,
  OPT_IGN_EXIT,
  WRAP_OPTION_SIZE
};
typedef enum Binary_Option Binary_Option;
static unsigned binary_option_size = WRAP_OPTION_SIZE;
static const char *option_str[] = {
  "intercept",
  "stats",
  "sql",
  "gory",
  "before",
  "after",
  "help",
  "orat",
  "frame",
  "time",
  "oci",
  "leak",
  "ign_exit"
};

struct Options {
  bool binary[WRAP_OPTION_SIZE];
  char filename[128];
  char time_format[32];
};
typedef struct Options Options;

static Options options = {
  .time_format = "%F_%H:%M:%S.#S:#_",
  //0
};

static int pp_stats()
{
  int ret = stats_sum_up(&stats, WRAP_STATEMENT_TYPE_SIZE, FN_SIZE, 0);
  IFTRUERET(ret, 0, ret);
  if (options.binary[OPT_INTERCEPT]) {
    ret = pp_stats_sql();
    IFTRUERET(ret, 0, ret);
    tprintf("\n\n");
    ret = stats_pp_fns(&stats, func_str, FN_SIZE);
    IFTRUERET(ret, 0, ret);
  }
  if (options.binary[OPT_OCI]) {
    ret = ocitrace_pp_stats(&stats.prog_time);
    IFTRUERET(ret, 0, ret);
  }
  return 0;
}


static int parse_env()
{
  int code = 0;
  const char *e = getenv("TRACEPROC_OPTIONS");
  if (!e)
    return 0;
  char s[256] = {0};
  strncpy(s, e, 255);
  size_t n = strlen(s);
  if (n)
    --n;
  for (char *t = strtok(s, " "); t; t=strtok(0, " ")) {
    bool found = false;
    for (unsigned i = 0; i<binary_option_size; ++i) {
      char *o = t;
      if (*o != '-')
        continue;
      ++o;
      bool no = false;
      if (!strncmp(o, "no", 2)) {
        no = true;
        o+=2;
        if (*o == '-')
          ++o;
      }
      if (!strcmp(option_str[i], o)) {
        found = true;
        options.binary[i] = !no;
      }
    }
    if (!found) {
      if (!strncmp(t, "-f", 2)) {
        strncpy(options.filename, t+2, 127);
        found = true;
      } else if (!strncmp(t, "-t", 2)) {
        strncpy(options.time_format, t+2, 31);
        options.binary[OPT_TIME] = true;
        found = true;
      }
    }
    if (!found) {
      fprintf(stderr, "Unknown option: %s\n", t);
      code = 1;
    }
  }
  return code;
}


#define WRAP_CALLBACKS_SIZE 10
struct State {
  Statement_Type last_stmt_code;
  const char *last_prepared_stmt;
  struct oraca *last_oraca;
  Traceproc_Callbacks callbacks[WRAP_CALLBACKS_SIZE];
  bool callbacks_present[WRAP_CALLBACKS_SIZE];
  unsigned callbacks_registered;

  FILE *file;
};
typedef struct State State;

static State state = {0};



int traceproc_register_callbacks(
    const Traceproc_Callbacks *callbacks,
    unsigned *id)
{
  for (unsigned i = 0; i < state.callbacks_registered; ++i) {
    if (!state.callbacks_present[i]) {
      state.callbacks[i] = *callbacks;
      state.callbacks_present[i] = true;
      *id = i;
      return 0;
    }
  }
  if (state.callbacks_registered >= WRAP_CALLBACKS_SIZE)
    return -1;
  unsigned i = state.callbacks_registered;
  state.callbacks[i] = *callbacks;
  state.callbacks_present[i] = true;
  ++state.callbacks_registered;
  *id = i;
  return 0;
}


int traceproc_unregister_callbacks(
    unsigned id
    )
{
  if (id >= state.callbacks_registered)
    return -1;
  state.callbacks[id] = (const Traceproc_Callbacks) {0};
  state.callbacks_present[id] = false;
  if (id + 1 == state.callbacks_registered)
    --state.callbacks_registered;
  return 0;
}


static size_t determine_iterations(bool before, const Statement *stmt)
{

  size_t iterations = 1;
  // XXX perhaps change to two separate variables for in/out iterations

  // for CONNECT Statement 'iterations' seems to have different semantics
  //if (stmt->type == CONNECT)
  //  iterations = 1;
  //
  unsigned effective_stmt_type = stmt->type;
  if (stmt->type == EXECUTE && state.last_prepared_stmt) {
    if (   strstr(state.last_prepared_stmt, "insert")
        || strstr(state.last_prepared_stmt, "INSERT")
        )
      effective_stmt_type = INSERT;
  }
  switch (effective_stmt_type) {
    case INSERT:
      iterations = stmt->iterations;
      break;
    case FETCH:
    case SELECT:
      if (before)
        iterations = 1;
      else {
        size_t mod = stmt->acc_fetched_rows % stmt->iterations;
        if (mod)
          iterations = mod;
        else
          iterations = stmt->iterations;
      }
      break;
    default:
      break;
  }
  return iterations;
}

static int pp_para(const sqlexd *d, bool before,
    const Statement *stmt,
    unsigned callback_nr)
{
  Parameter p = {0};

  p.iterations = determine_iterations(before, stmt);

  for (size_t iter = 0; iter < p.iterations; ++iter) {
    p.iteration = iter;

    size_t i = 0;
    size_t a = 0;
    const short *begin = d->cud + d->offset + 15;
    size_t x = stmt->number_of_params * 4;
    for (; i < x; i+=4, ++a) {
      p.pos = a;

      // size is used as increment in array inserts
      p.size = d->sqphss[a]; // == sqhsts
      p.ind_size = d->sqpins[a]; // == sqinds

      size_t mult = iter;

      switch (begin[i]) {
        case 1:
          p.direction = PARA_IN;

          // SELECT stmts don't have input arrays
          if (stmt->type == SELECT && iter > 0)
            mult = 0;
          break;
        case 2:
          p.direction = PARA_OUT;
          break;
        default:
          fprintf(stderr, "Unknown para marker: %d\n", begin[i]);
          return -1;
      }
      p.value = d->sqphsv[a] + mult * p.size; // == sqhstv
      p.length = d->sqphsl[a]; // == sqhstl
      p.indicator = d->sqpind[a]
        ? * (short*) (((const void*)d->sqpind[a]) + mult * p.ind_size)
        : 0 ; // == sqindv
      p.type = begin[i+1];
      p.type_str = type_to_str(p.type);

      if (stmt->type == PREPARE && p.direction == PARA_IN
          && (p.type == STRING || p.type == STRING0)
          ) {
          state.last_prepared_stmt = p.value;
      }

      // don't print OUT-parameters on errors
      if (!before && p.direction == PARA_OUT
            && !(!stmt->errorcode ) //|| stmt->errorcode == 1403 || stmt->errorcode == 100)
            ) {
        p.indicator = -1;
      }

      if (state.callbacks[callback_nr].parameter_fn) {
        int ret = state.callbacks[callback_nr]
          .parameter_fn(stmt, &p, before,
              state.callbacks[callback_nr].user_ptr);
      }
    }
  }
  return 0;
}

static const char *immediate_stmt(const sqlexd *d, const Statement *stmt)
{
    const short *begin = d->cud + d->offset + 15;
    if (begin[0] !=  1)
      return 0;

    if (!(begin[1] == STRING  || begin[1] == STRING0))
      return 0;
    const void *value = d->sqphsv[0];
    return value;
}

static int pp_sql_before(const sqlexd *d, Statement *stmt, unsigned callback_nr)
{
  const short *cud = d->cud + d->offset;
  stmt->offset = cud[0];

  if (stmt->offset != d->offset)
  {
    fprintf(stderr, "Stored offset (%d) != reference one (%d)\n",
        stmt->offset, d->offset);
    return -1;
  }

  stmt->number = cud[3];
  stmt->length = cud[4];
  stmt->type = cud[6];
  stmt->type_str = fn_to_str(stmt->type);
  short low_line_number = cud[7];
  short hi_line_number  = cud[8];
  stmt->line_number =    hi_line_number * 1024 * 2 * 2 * 2 // 2^13
    + low_line_number;
  stmt->iterations = d->iters;
  stmt->number_of_params = cud[10];
  stmt->number_of_params_in = cud[11];
  if (stmt->type == CONNECT) {
    stmt->max_row_insert = stmt->number;
    --stmt->number_of_params;
    --stmt->number_of_params_in;
  }

  switch (stmt->type) {
    case OPEN:
    case OPEN_DYNAMIC:
      // not necessarily initialized for other stmts
      stmt->scroll_cursor = d->sqcmod;
      break;
    case FETCH:
      stmt->fetch_type = (Fetch_Type) d->sqfmod;
      stmt->fetch_type_str = fetch_to_str(stmt->fetch_type);
      stmt->fetch_offset = d->sqfoff;
      break;
    default:
      break;
  }

  if (stmt->length)
    stmt->text = d->stmt;
  else {
    stmt->text = default_sql_text(stmt->type);
  }

  if (stmt->type == OPEN || stmt->type == FETCH)
    stmt->prefetch = d->selerr;

  // XXX fix for multiple prepared statements
  if (stmt->type == EXECUTE)
    stmt->last_prepared_stmt = state.last_prepared_stmt;
  if (stmt->type == EXECUTE_IMMEDIATE)
    stmt->immediate_stmt = immediate_stmt(d, stmt);

  traceproc_trap(stmt->type_str, stmt->text, true, true);

  // printing a FETCH statement before execution
  // is not really useful
  if (stmt->type == FETCH && options.binary[OPT_SQL])
    return 0;


  if (state.callbacks[callback_nr].before_fn) {
    int ret = state.callbacks[callback_nr]
      .before_fn(stmt, state.callbacks[callback_nr].user_ptr);
  }

  pp_para(d, true, stmt, callback_nr);

  if (state.callbacks[callback_nr].before_end_fn) {
    int ret = state.callbacks[callback_nr]
      .before_end_fn(stmt, state.callbacks[callback_nr].user_ptr);
  }

  return 0;
}

static int pp_sql_after(const sqlexd *d, Statement *stmt,
    unsigned callback_nr)
{
  const struct sqlca *s = (const struct sqlca *) d->sqlest;
  if (!s) {
    fprintf(stderr, "Could not find sqlca");
    return -1;
  }
  stmt->acc_fetched_rows = s->sqlerrd[2];
  stmt->errorcode = s->sqlcode;

  char msg[512] = "NOMSG";
  stmt->msg = msg;
  char extended_msg[2048] = {0};
  stmt->extended_msg = extended_msg;
  if (stmt->errorcode)
  {
    size_t size = 511;
    size_t ret = 0;
    sqlglm((unsigned char*)msg, &size, &ret);
    msg[ret] = 0;
    if (ret && msg[ret-1] == '\n')
      msg[ret-1] = 0;

    if (state.last_oraca) {
      size_t n = 2047;
      if (state.last_oraca->orastxt.orastxtl < 2047)
        n = state.last_oraca->orastxt.orastxtl;
      strncpy(extended_msg, state.last_oraca->orastxt.orastxtc, n);
    }
  }

  traceproc_trap(stmt->type_str, stmt->text, true, false);

  if (state.callbacks[callback_nr].after_fn) {
    int ret = state.callbacks[callback_nr]
      .after_fn(stmt, s, state.last_oraca,
          state.callbacks[callback_nr].user_ptr);
  }

  // now done in pp_para
  //if (!stmt->errorcode || stmt->errorcode == 1403 || stmt->errorcode == 100) {
    pp_para(d, false, stmt, callback_nr);
  //}

  if (state.callbacks[callback_nr].after_end_fn) {
    int ret = state.callbacks[callback_nr]
      .after_end_fn(stmt, state.callbacks[callback_nr].user_ptr);
  }

  return 0;
}

struct Fn_Table {
  void (*sqlcxt)(void**, unsigned int*, sqlexd *, const sqlcxp*);
  void (*sqlorat)(void **, unsigned int *, void *);
  void (*_exit)(int status);
  void (*_Exit)(int status);
  //void (*exit)(int status);
};
typedef struct Fn_Table Fn_Table;
static Fn_Table fn_table = {0};

INTERCEPT_SETUP(sqlcxt)
INTERCEPT_SETUP(sqlorat)
INTERCEPT_SETUP(_exit)
INTERCEPT_SETUP(_Exit)
//INTERCEPT_SETUP(exit)

static void wrap_exit(int status)
{
  exit(status);
  /*
  if (fn_table.exit)
    (*fn_table.exit)(status);
  else
    exit(status);
   */
}

static void setup_fns()
{
  int r = 0;
  r += setup_sqlcxt();
  r += setup_sqlorat();
  r += setup__exit();
  r += setup__Exit();
  //r += setup_exit();
  if (r) {
    fprintf(stderr, "Exiting because of previous dlsym() errors.\n");
    wrap_exit(23);
  }
}

// libclntsh.so
void sqlcxt (void **v, unsigned int * i,
                    sqlexd *d, const sqlcxp *p)
{
  Statement stmt = {0};
  stmt.filename = p->filnam;
  if (options.binary[OPT_INTERCEPT]) {

    if (options.binary[OPT_FRAME])
      tprintf("Calling oracle sqlcxt() ...\n");
    if (options.binary[OPT_BEFORE])
      for (unsigned i = 0; i < state.callbacks_registered; ++i)
        pp_sql_before(d, &stmt, i);
  }

  STATS_BEGIN(options.binary[OPT_STATS]);

  (*fn_table.sqlcxt)(v, i, d, p);

  STATS_END(options.binary[OPT_STATS], FN_SQLCXT, stmt.type);

  if (options.binary[OPT_INTERCEPT]) {
    if (options.binary[OPT_AFTER])
      for (unsigned i = 0; i < state.callbacks_registered; ++i)
        pp_sql_after(d, &stmt, i);

    if (options.binary[OPT_FRAME]) {
      tprintf("\n");
      tprintf("Calling oracle sqlcxt() ... done\n\n");
    }

  }
}

void sqlorat(void **v, unsigned int *sqlctx_, void *oraca_)
{
  if (options.binary[OPT_INTERCEPT]) {
    if (options.binary[OPT_FRAME] && options.binary[OPT_ORAT])
      tprintf("Calling oracle sqlorat() ...\n");
    state.last_oraca = oraca_;
  }

  STATS_BEGIN(options.binary[OPT_STATS]);

  (*fn_table.sqlorat)(v, sqlctx_, oraca_);

  STATS_END(options.binary[OPT_STATS], FN_SQLORAT, 0);

  if (options.binary[OPT_INTERCEPT]) {

    if (options.binary[OPT_FRAME] && options.binary[OPT_ORAT]) {
      tprintf("\n");
      tprintf("Calling oracle sqlorat() ... done\n\n");
    }
  }
}

/*
void exit(int status)
{
  (*fn_table.exit)(status);
  abort();
}
*/

/* Because the flushing of IO-Buffers on the executation
 * of destructor function on _exit() is implementation
 * defined one might want to use -ign_exit in special
 * cases.
 */
void _exit(int status)
{
  if (options.binary[OPT_IGN_EXIT]){
    //(*fn_table.exit)(status);
    exit(status);
  } else
    (*fn_table._exit)(status);
  // to remove Warning
  abort();
}

void _Exit(int status)
{
  if (options.binary[OPT_IGN_EXIT])
    //(*fn_table.exit)(status);
    exit(status);
  else
    (*fn_table._Exit)(status);
  // to remove Warning
  abort();
}

// not necessary anymore - Solaris CC 12.3 also supports the attribute-syntax
// for constructor/destructor declarations
//#if defined(__sun)
//#pragma init(wrap_startup)
//#pragma fini(wrap_shutdown)
//#endif

static void help()
{
  fprintf(stderr, "libtraceproc - a tracing library for Oracle Pro*C/OCI\n"
      "\n"
      "Call: $ TRACEPROC_OPTIONS=\"-opt1 ...\" LD_PRELOAD=/path/libtraceproc.so\n\n"
      "Options:\n\n"
      "  -help        - this screen\n"
      "  -fFILENAME   - write trace to FILENAME (default: stderr)\n"
      "  -tFORMAT     - timestamp strftime-like string (default: %%F_%%H:%%M:%%S.#S:#_)\n"
      "  -intercept   - enable Pro*C tracing (default: no)\n"
      "  -oci         - also trace OCI calls (default: no)\n"
      "  -leak        - check for handle/desc/... leaks - iff -oci is enabled\n\t\t\t\t\t (default: yes)\n"
      "  -stats       - print statistics at the end (default: yes)\n"
      "  -sql         - print complete SQL-statements (default: yes)\n"
      "  -gory        - print gory details - verbose mode (default: no)\n"
      "  -before      - call pretty printers before (default: yes)\n"
      "  -after       - call pretty printers after (default: yes)\n"
      "  -orat        - intercept sqlorat calls (default: yes)\n"
      "  -frame       - print 'starting sqlctx()/done' msg (default: no)\n"
      "  -time        - print timestamps (default: yes)\n"
      "  -ign_exit    - map _exit()/_Exit() to exit() (default: no)\n"
      "\n"
      "\n"
      "You can prefix each binary option with -no, e.g. -nostats,\n"
      "to disable it.\n"
      "\n\n"
      "2013-06-30, Georg Sauthoff <mail@georg.so>, GPLv3+, Version 0.5\n"
      "\n");
}

static SQL_PP_State sql_pp_state = {0};

static void init_default_options()
{
  options.binary[OPT_INTERCEPT] = false;
  options.binary[OPT_STATS] = true;
  options.binary[OPT_GORY] = false;
  options.binary[OPT_SQL] = true;
  options.binary[OPT_BEFORE] = true;
  options.binary[OPT_AFTER] = true;
  options.binary[OPT_TIME] = true;
  options.binary[OPT_OCI] = false;
  options.binary[OPT_LEAK] = true;
}

static void interpret_options()
{
  if (!options.binary[OPT_TIME])
    *options.time_format = 0;
  if (*options.time_format) {
    trace_set_ftime(options.time_format);
  }
  if (*options.filename) {
    state.file = fopen(options.filename, "w");
    if (!state.file) {
      fprintf(stderr, "Could not open: %s => %s\n",
          options.filename, strerror(errno));
      wrap_exit(2);
    }
  } else {
    state.file = stderr;
  }
}

static void setup_callbacks()
{
  if (options.binary[OPT_GORY]) {
    Traceproc_Callbacks callbacks = {
        .parameter_fn = pp_para_gory,
        .before_fn = pp_before_gory,
        .before_end_fn = pp_before_after_end_gory,
        .after_fn = pp_after_gory,
        .after_end_fn = pp_before_after_end_gory
    };
    unsigned id = 0;
    traceproc_register_callbacks(&callbacks, &id);
  }
  if (options.binary[OPT_SQL]) {
    Traceproc_Callbacks callbacks = {
        .user_ptr = &sql_pp_state,

        .parameter_fn = pp_para_sql,
        .before_fn = pp_before_sql,
        .before_end_fn = pp_before_after_end_sql,
        .after_fn = pp_after_sql,
        .after_end_fn = pp_before_after_end_sql
    };
    unsigned id = 0;
    traceproc_register_callbacks(&callbacks, &id);
  }
}

static void print_startup()
{
  if (options.binary[OPT_INTERCEPT]) {
    int ret = tprintf("Libtraceproc is active.\n");
    if (ret <= 0) {
      fprintf(stderr, "libtraceproc stdio failed");
      wrap_exit(3);
    }
    const char *e = getenv("TRACEPROC_OPTIONS");
    tprintf("TRACEPROC_OPTIONS='%s'\n", e ? e : "");
  }
}

static void __attribute__((constructor)) wrap_startup() 
{
  init_default_options();
  int ret_p = parse_env();
  if (options.binary[OPT_HELP] || ret_p) {
    help();
    wrap_exit(1);
  }
  interpret_options();
  setup_fns();
  ocitrace_setup(options.binary[OPT_OCI],
      options.binary[OPT_GORY], options.binary[OPT_SQL],
      options.binary[OPT_STATS],
      options.binary[OPT_LEAK]);
  int ret = stats_init(&stats, WRAP_STATEMENT_TYPE_SIZE, FN_SIZE);
  IFTRUEEXIT(ret, 0, -1);

  trace_set_file(state.file);

  setup_callbacks();

  if (options.binary[OPT_STATS]) {
    ret = clock_gettime(WRAP_CLOCK_ID, &stats.prog_start);
    IFERRNOEXIT(ret, 0, 10);
  }

  print_startup();
}

static void __attribute__((destructor)) wrap_shutdown()
{
  if (options.binary[OPT_OCI])
    ocitrace_finish();

  if (options.binary[OPT_STATS]) {
    int ret = clock_gettime(WRAP_CLOCK_ID, &stats.prog_end);
    IFERRNOEXIT(ret, 0, 10);

    ret = timespec_add_intv(&stats.prog_time,
        &stats.prog_start, &stats.prog_end);
    IFTRUEEXIT(ret, 0, 10);

    pp_stats();
  }
}

void traceproc_set_intercept(bool b)
{
  options.binary[OPT_INTERCEPT] = b;
}

void traceproc_set_oci(bool b)
{
  options.binary[OPT_OCI] = b;
}

