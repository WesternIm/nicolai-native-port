
/mnt/data/m27_work/msi_extract/flat/mtsyc32.dll:     file format pei-i386


Disassembly of section .text:

10215620 <.text+0x214620>:
10215620:	10 50 ff             	adc    BYTE PTR [eax-0x1],dl
10215623:	15 54 a1 23 10       	adc    eax,0x1023a154
10215628:	8d 8c 24 c0 00 00 00 	lea    ecx,[esp+0xc0]
1021562f:	56                   	push   esi
10215630:	8d 54 24 60          	lea    edx,[esp+0x60]
10215634:	51                   	push   ecx
10215635:	52                   	push   edx
10215636:	e8 65 96 f8 ff       	call   0x1019eca0
1021563b:	83 c4 18             	add    esp,0x18
1021563e:	8b 74 24 28          	mov    esi,DWORD PTR [esp+0x28]
10215642:	8b bc 24 48 02 00 00 	mov    edi,DWORD PTR [esp+0x248]
10215649:	8d 84 24 b4 00 00 00 	lea    eax,[esp+0xb4]
10215650:	50                   	push   eax
10215651:	56                   	push   esi
10215652:	57                   	push   edi
10215653:	e8 d8 8e f8 ff       	call   0x1019e530
10215658:	8b 8f 3c 01 00 00    	mov    ecx,DWORD PTR [edi+0x13c]
1021565e:	33 c0                	xor    eax,eax
10215660:	83 c4 0c             	add    esp,0xc
10215663:	8d 94 24 18 01 00 00 	lea    edx,[esp+0x118]
1021566a:	8b 3c b1             	mov    edi,DWORD PTR [ecx+esi*4]
1021566d:	83 c9 ff             	or     ecx,0xffffffff
10215670:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
10215672:	f7 d1                	not    ecx
10215674:	2b f9                	sub    edi,ecx
10215676:	c7 44 24 10 00 00 00 	mov    DWORD PTR [esp+0x10],0x0
1021567d:	00 
1021567e:	8b c1                	mov    eax,ecx
10215680:	8b f7                	mov    esi,edi
10215682:	8b fa                	mov    edi,edx
10215684:	c1 e9 02             	shr    ecx,0x2
10215687:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
10215689:	8b c8                	mov    ecx,eax
1021568b:	8b 44 24 30          	mov    eax,DWORD PTR [esp+0x30]
1021568f:	83 e1 03             	and    ecx,0x3
10215692:	f3 a4                	rep movs BYTE PTR es:[edi],BYTE PTR ds:[esi]
10215694:	8b 8c 24 48 02 00 00 	mov    ecx,DWORD PTR [esp+0x248]
1021569b:	8d bc 24 18 01 00 00 	lea    edi,[esp+0x118]
102156a2:	8b 91 30 01 00 00    	mov    edx,DWORD PTR [ecx+0x130]
102156a8:	0f be 0c 02          	movsx  ecx,BYTE PTR [edx+eax*1]
102156ac:	8b 54 24 40          	mov    edx,DWORD PTR [esp+0x40]
102156b0:	8d 04 cd 00 00 00 00 	lea    eax,[ecx*8+0x0]
102156b7:	2b c1                	sub    eax,ecx
102156b9:	8d 0c 40             	lea    ecx,[eax+eax*2]
102156bc:	8b 82 58 12 08 00    	mov    eax,DWORD PTR [edx+0x81258]
102156c2:	8d 74 48 12          	lea    esi,[eax+ecx*2+0x12]
102156c6:	83 c9 ff             	or     ecx,0xffffffff
102156c9:	33 c0                	xor    eax,eax
102156cb:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
102156cd:	f7 d1                	not    ecx
102156cf:	49                   	dec    ecx
102156d0:	85 c9                	test   ecx,ecx
102156d2:	0f 8e 9a 01 00 00    	jle    0x10215872
102156d8:	bf 01 00 00 00       	mov    edi,0x1
102156dd:	89 7c 24 2c          	mov    DWORD PTR [esp+0x2c],edi
102156e1:	eb 05                	jmp    0x102156e8
102156e3:	bf 01 00 00 00       	mov    edi,0x1
102156e8:	8b 4c 24 2c          	mov    ecx,DWORD PTR [esp+0x2c]
102156ec:	a1 74 fe 72 10       	mov    eax,ds:0x1072fe74
102156f1:	0f be 94 0c 17 01 00 	movsx  edx,BYTE PTR [esp+ecx*1+0x117]
102156f8:	00 
102156f9:	52                   	push   edx
102156fa:	50                   	push   eax
102156fb:	ff 15 1c a2 23 10    	call   DWORD PTR ds:0x1023a21c
10215701:	83 c4 08             	add    esp,0x8
10215704:	85 c0                	test   eax,eax
10215706:	0f 84 3b 01 00 00    	je     0x10215847
1021570c:	8b 44 24 2c          	mov    eax,DWORD PTR [esp+0x2c]
10215710:	80 bc 04 18 01 00 00 	cmp    BYTE PTR [esp+eax*1+0x118],0x3e
10215717:	3e 
10215718:	0f 85 a7 02 00 00    	jne    0x102159c5
1021571e:	3b dd                	cmp    ebx,ebp
10215720:	75 1a                	jne    0x1021573c
10215722:	0f be 46 02          	movsx  eax,BYTE PTR [esi+0x2]
10215726:	0f be 7e 07          	movsx  edi,BYTE PTR [esi+0x7]
1021572a:	0f be 4e 0e          	movsx  ecx,BYTE PTR [esi+0xe]
1021572e:	03 f8                	add    edi,eax
10215730:	03 c8                	add    ecx,eax
10215732:	89 4c 24 1c          	mov    DWORD PTR [esp+0x1c],ecx
10215736:	0f be 4e 15          	movsx  ecx,BYTE PTR [esi+0x15]
1021573a:	eb 48                	jmp    0x10215784
1021573c:	8b 44 24 44          	mov    eax,DWORD PTR [esp+0x44]
10215740:	85 c0                	test   eax,eax
10215742:	0f 85 7a 01 00 00    	jne    0x102158c2
10215748:	8b 8c 24 48 02 00 00 	mov    ecx,DWORD PTR [esp+0x248]
1021574f:	89 7c 24 44          	mov    DWORD PTR [esp+0x44],edi
10215753:	db 44 24 18          	fild   DWORD PTR [esp+0x18]
10215757:	8b 91 04 3d 00 00    	mov    edx,DWORD PTR [ecx+0x3d04]
1021575d:	d8 5a 60             	fcomp  DWORD PTR [edx+0x60]
10215760:	df e0                	fnstsw ax
10215762:	f6 c4 41             	test   ah,0x41
10215765:	75 06                	jne    0x1021576d
10215767:	0f be 46 01          	movsx  eax,BYTE PTR [esi+0x1]
1021576b:	eb 03                	jmp    0x10215770
1021576d:	0f be 06             	movsx  eax,BYTE PTR [esi]
10215770:	0f be 7e 03          	movsx  edi,BYTE PTR [esi+0x3]
10215774:	0f be 4e 0a          	movsx  ecx,BYTE PTR [esi+0xa]
10215778:	03 f8                	add    edi,eax
1021577a:	03 c8                	add    ecx,eax
1021577c:	89 4c 24 1c          	mov    DWORD PTR [esp+0x1c],ecx
10215780:	0f be 4e 11          	movsx  ecx,BYTE PTR [esi+0x11]
10215784:	03 c8                	add    ecx,eax
10215786:	89 4c 24 20          	mov    DWORD PTR [esp+0x20],ecx
1021578a:	57                   	push   edi
1021578b:	8d 4c 24 54          	lea    ecx,[esp+0x54]
1021578f:	68 5c cf 75 10       	push   0x1075cf5c
10215794:	51                   	push   ecx
10215795:	ff 15 54 a1 23 10    	call   DWORD PTR ds:0x1023a154
1021579b:	8b 54 24 38          	mov    edx,DWORD PTR [esp+0x38]
1021579f:	8d 84 24 24 01 00 00 	lea    eax,[esp+0x124]
102157a6:	52                   	push   edx
102157a7:	8d 4c 24 60          	lea    ecx,[esp+0x60]
102157ab:	50                   	push   eax
102157ac:	51                   	push   ecx
102157ad:	e8 ee 94 f8 ff       	call   0x1019eca0
102157b2:	8d 7c 24 68          	lea    edi,[esp+0x68]
102157b6:	83 c9 ff             	or     ecx,0xffffffff
102157b9:	33 c0                	xor    eax,eax
102157bb:	8b 54 24 34          	mov    edx,DWORD PTR [esp+0x34]
102157bf:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
102157c1:	f7 d1                	not    ecx
102157c3:	49                   	dec    ecx
102157c4:	52                   	push   edx
102157c5:	8d 44 24 6c          	lea    eax,[esp+0x6c]
102157c9:	8b f9                	mov    edi,ecx
102157cb:	68 5c cf 75 10       	push   0x1075cf5c
102157d0:	50                   	push   eax
102157d1:	89 7c 24 6c          	mov    DWORD PTR [esp+0x6c],edi
102157d5:	ff 15 54 a1 23 10    	call   DWORD PTR ds:0x1023a154
102157db:	8b 4c 24 34          	mov    ecx,DWORD PTR [esp+0x34]
102157df:	8d 84 24 3c 01 00 00 	lea    eax,[esp+0x13c]
102157e6:	8d 54 39 01          	lea    edx,[ecx+edi*1+0x1]
102157ea:	8d 4c 24 74          	lea    ecx,[esp+0x74]
102157ee:	52                   	push   edx
102157ef:	50                   	push   eax
102157f0:	51                   	push   ecx
102157f1:	e8 aa 94 f8 ff       	call   0x1019eca0
102157f6:	8d bc 24 80 00 00 00 	lea    edi,[esp+0x80]
102157fd:	83 c9 ff             	or     ecx,0xffffffff
10215800:	33 c0                	xor    eax,eax
10215802:	8b 54 24 50          	mov    edx,DWORD PTR [esp+0x50]
10215806:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
10215808:	8b 7c 24 78          	mov    edi,DWORD PTR [esp+0x78]
1021580c:	52                   	push   edx
1021580d:	f7 d1                	not    ecx
1021580f:	8d 84 24 84 00 00 00 	lea    eax,[esp+0x84]
10215816:	49                   	dec    ecx
10215817:	68 5c cf 75 10       	push   0x1075cf5c
1021581c:	50                   	push   eax
1021581d:	03 f9                	add    edi,ecx
1021581f:	ff 15 54 a1 23 10    	call   DWORD PTR ds:0x1023a154
10215825:	8b 4c 24 4c          	mov    ecx,DWORD PTR [esp+0x4c]
10215829:	8d 84 24 54 01 00 00 	lea    eax,[esp+0x154]
10215830:	8d 54 39 01          	lea    edx,[ecx+edi*1+0x1]
10215834:	8d 8c 24 8c 00 00 00 	lea    ecx,[esp+0x8c]
1021583b:	52                   	push   edx
1021583c:	50                   	push   eax
1021583d:	51                   	push   ecx
1021583e:	e8 5d 94 f8 ff       	call   0x1019eca0
10215843:	83 c4 48             	add    esp,0x48
10215846:	45                   	inc    ebp
10215847:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
1021584b:	8b 4c 24 2c          	mov    ecx,DWORD PTR [esp+0x2c]
1021584f:	42                   	inc    edx
10215850:	41                   	inc    ecx
10215851:	89 4c 24 2c          	mov    DWORD PTR [esp+0x2c],ecx
10215855:	8d bc 24 18 01 00 00 	lea    edi,[esp+0x118]
1021585c:	83 c9 ff             	or     ecx,0xffffffff
1021585f:	33 c0                	xor    eax,eax
10215861:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
10215863:	f7 d1                	not    ecx
10215865:	49                   	dec    ecx
10215866:	89 54 24 10          	mov    DWORD PTR [esp+0x10],edx
1021586a:	3b d1                	cmp    edx,ecx
1021586c:	0f 8c 71 fe ff ff    	jl     0x102156e3
10215872:	8b 7c 24 28          	mov    edi,DWORD PTR [esp+0x28]
10215876:	8b b4 24 48 02 00 00 	mov    esi,DWORD PTR [esp+0x248]
1021587d:	8d 94 24 18 01 00 00 	lea    edx,[esp+0x118]
10215884:	52                   	push   edx
10215885:	57                   	push   edi
10215886:	56                   	push   esi
10215887:	e8 a4 8c f8 ff       	call   0x1019e530
1021588c:	8b 44 24 3c          	mov    eax,DWORD PTR [esp+0x3c]
10215890:	83 c4 0c             	add    esp,0xc
10215893:	47                   	inc    edi
10215894:	3b f8                	cmp    edi,eax
10215896:	89 7c 24 28          	mov    DWORD PTR [esp+0x28],edi
1021589a:	0f 8e 03 f9 ff ff    	jle    0x102151a3
102158a0:	8b 44 24 30          	mov    eax,DWORD PTR [esp+0x30]
102158a4:	8b 8e 34 01 00 00    	mov    ecx,DWORD PTR [esi+0x134]
102158aa:	40                   	inc    eax
102158ab:	3b c1                	cmp    eax,ecx
102158ad:	89 44 24 1c          	mov    DWORD PTR [esp+0x1c],eax
102158b1:	0f 8e ff f6 ff ff    	jle    0x10214fb6
102158b7:	5f                   	pop    edi
102158b8:	5d                   	pop    ebp
102158b9:	5b                   	pop    ebx
102158ba:	5e                   	pop    esi
102158bb:	81 c4 34 02 00 00    	add    esp,0x234
102158c1:	c3                   	ret
102158c2:	8b 84 24 48 02 00 00 	mov    eax,DWORD PTR [esp+0x248]
102158c9:	53                   	push   ebx
102158ca:	db 44 24 1c          	fild   DWORD PTR [esp+0x1c]
102158ce:	8b 88 04 3d 00 00    	mov    ecx,DWORD PTR [eax+0x3d04]
102158d4:	55                   	push   ebp
102158d5:	d8 59 60             	fcomp  DWORD PTR [ecx+0x60]
102158d8:	df e0                	fnstsw ax
102158da:	f6 c4 41             	test   ah,0x41
102158dd:	75 75                	jne    0x10215954
102158df:	8a 56 02             	mov    dl,BYTE PTR [esi+0x2]
102158e2:	8a 44 24 1c          	mov    al,BYTE PTR [esp+0x1c]
102158e6:	8a 4e 05             	mov    cl,BYTE PTR [esi+0x5]
102158e9:	52                   	push   edx
102158ea:	02 46 01             	add    al,BYTE PTR [esi+0x1]
102158ed:	50                   	push   eax
102158ee:	51                   	push   ecx
102158ef:	e8 3c 04 00 00       	call   0x10215d30
102158f4:	8a 56 02             	mov    dl,BYTE PTR [esi+0x2]
102158f7:	8a 4e 0c             	mov    cl,BYTE PTR [esi+0xc]
102158fa:	53                   	push   ebx
102158fb:	55                   	push   ebp
102158fc:	8b f8                	mov    edi,eax
102158fe:	8a 44 24 30          	mov    al,BYTE PTR [esp+0x30]
10215902:	52                   	push   edx
10215903:	8a 56 01             	mov    dl,BYTE PTR [esi+0x1]
10215906:	02 c2                	add    al,dl
10215908:	50                   	push   eax
10215909:	51                   	push   ecx
1021590a:	e8 21 04 00 00       	call   0x10215d30
1021590f:	8a 56 02             	mov    dl,BYTE PTR [esi+0x2]
10215912:	8a 4e 13             	mov    cl,BYTE PTR [esi+0x13]
10215915:	53                   	push   ebx
10215916:	55                   	push   ebp
10215917:	89 44 24 4c          	mov    DWORD PTR [esp+0x4c],eax
1021591b:	8a 44 24 44          	mov    al,BYTE PTR [esp+0x44]
1021591f:	52                   	push   edx
10215920:	8a 56 01             	mov    dl,BYTE PTR [esi+0x1]
10215923:	02 c2                	add    al,dl
10215925:	50                   	push   eax
10215926:	51                   	push   ecx
10215927:	e8 04 04 00 00       	call   0x10215d30
1021592c:	89 44 24 5c          	mov    DWORD PTR [esp+0x5c],eax
10215930:	8b 44 24 74          	mov    eax,DWORD PTR [esp+0x74]
10215934:	83 c4 3c             	add    esp,0x3c
10215937:	85 c0                	test   eax,eax
10215939:	0f 84 4b fe ff ff    	je     0x1021578a
1021593f:	8a 44 24 14          	mov    al,BYTE PTR [esp+0x14]
10215943:	8a 4e 01             	mov    cl,BYTE PTR [esi+0x1]
10215946:	8a 56 02             	mov    dl,BYTE PTR [esi+0x2]
10215949:	53                   	push   ebx
1021594a:	55                   	push   ebp
1021594b:	02 c1                	add    al,cl
1021594d:	52                   	push   edx
1021594e:	50                   	push   eax
1021594f:	e9 57 03 00 00       	jmp    0x10215cab
10215954:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215957:	8a 54 24 1c          	mov    dl,BYTE PTR [esp+0x1c]
1021595b:	8a 46 05             	mov    al,BYTE PTR [esi+0x5]
1021595e:	51                   	push   ecx
1021595f:	02 16                	add    dl,BYTE PTR [esi]
10215961:	52                   	push   edx
10215962:	50                   	push   eax
10215963:	e8 c8 03 00 00       	call   0x10215d30
10215968:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
1021596b:	8a 54 24 28          	mov    dl,BYTE PTR [esp+0x28]
1021596f:	53                   	push   ebx
10215970:	55                   	push   ebp
10215971:	51                   	push   ecx
10215972:	8a 0e                	mov    cl,BYTE PTR [esi]
10215974:	8b f8                	mov    edi,eax
10215976:	8a 46 0c             	mov    al,BYTE PTR [esi+0xc]
10215979:	02 d1                	add    dl,cl
1021597b:	52                   	push   edx
1021597c:	50                   	push   eax
1021597d:	e8 ae 03 00 00       	call   0x10215d30
10215982:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215985:	8a 54 24 3c          	mov    dl,BYTE PTR [esp+0x3c]
10215989:	53                   	push   ebx
1021598a:	55                   	push   ebp
1021598b:	51                   	push   ecx
1021598c:	8a 0e                	mov    cl,BYTE PTR [esi]
1021598e:	89 44 24 50          	mov    DWORD PTR [esp+0x50],eax
10215992:	8a 46 13             	mov    al,BYTE PTR [esi+0x13]
10215995:	02 d1                	add    dl,cl
10215997:	52                   	push   edx
10215998:	50                   	push   eax
10215999:	e8 92 03 00 00       	call   0x10215d30
1021599e:	89 44 24 5c          	mov    DWORD PTR [esp+0x5c],eax
102159a2:	8b 44 24 74          	mov    eax,DWORD PTR [esp+0x74]
102159a6:	83 c4 3c             	add    esp,0x3c
102159a9:	85 c0                	test   eax,eax
102159ab:	0f 84 d9 fd ff ff    	je     0x1021578a
102159b1:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
102159b4:	8a 54 24 14          	mov    dl,BYTE PTR [esp+0x14]
102159b8:	53                   	push   ebx
102159b9:	55                   	push   ebp
102159ba:	51                   	push   ecx
102159bb:	8a 0e                	mov    cl,BYTE PTR [esi]
102159bd:	02 d1                	add    dl,cl
102159bf:	52                   	push   edx
102159c0:	e9 e6 02 00 00       	jmp    0x10215cab
102159c5:	68 78 cf 75 10       	push   0x1075cf78
102159ca:	6a 02                	push   0x2
102159cc:	50                   	push   eax
102159cd:	8d 84 24 24 01 00 00 	lea    eax,[esp+0x124]
102159d4:	50                   	push   eax
102159d5:	e8 c6 93 f8 ff       	call   0x1019eda0
102159da:	83 c4 10             	add    esp,0x10
102159dd:	85 c0                	test   eax,eax
102159df:	0f 85 02 01 00 00    	jne    0x10215ae7
102159e5:	8b 8c 24 48 02 00 00 	mov    ecx,DWORD PTR [esp+0x248]
102159ec:	53                   	push   ebx
102159ed:	db 44 24 1c          	fild   DWORD PTR [esp+0x1c]
102159f1:	8b 91 04 3d 00 00    	mov    edx,DWORD PTR [ecx+0x3d04]
102159f7:	55                   	push   ebp
102159f8:	d8 5a 60             	fcomp  DWORD PTR [edx+0x60]
102159fb:	df e0                	fnstsw ax
102159fd:	f6 c4 41             	test   ah,0x41
10215a00:	75 74                	jne    0x10215a76
10215a02:	8a 4c 24 1c          	mov    cl,BYTE PTR [esp+0x1c]
10215a06:	8a 56 01             	mov    dl,BYTE PTR [esi+0x1]
10215a09:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215a0c:	02 ca                	add    cl,dl
10215a0e:	8a 56 09             	mov    dl,BYTE PTR [esi+0x9]
10215a11:	50                   	push   eax
10215a12:	51                   	push   ecx
10215a13:	52                   	push   edx
10215a14:	e8 17 03 00 00       	call   0x10215d30
10215a19:	8a 4c 24 28          	mov    cl,BYTE PTR [esp+0x28]
10215a1d:	8a 56 10             	mov    dl,BYTE PTR [esi+0x10]
10215a20:	8b f8                	mov    edi,eax
10215a22:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215a25:	53                   	push   ebx
10215a26:	55                   	push   ebp
10215a27:	50                   	push   eax
10215a28:	8a 46 01             	mov    al,BYTE PTR [esi+0x1]
10215a2b:	02 c8                	add    cl,al
10215a2d:	51                   	push   ecx
10215a2e:	52                   	push   edx
10215a2f:	e8 fc 02 00 00       	call   0x10215d30
10215a34:	8a 4c 24 3c          	mov    cl,BYTE PTR [esp+0x3c]
10215a38:	8a 56 17             	mov    dl,BYTE PTR [esi+0x17]
10215a3b:	89 44 24 44          	mov    DWORD PTR [esp+0x44],eax
10215a3f:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215a42:	53                   	push   ebx
10215a43:	55                   	push   ebp
10215a44:	50                   	push   eax
10215a45:	8a 46 01             	mov    al,BYTE PTR [esi+0x1]
10215a48:	02 c8                	add    cl,al
10215a4a:	51                   	push   ecx
10215a4b:	52                   	push   edx
10215a4c:	e8 df 02 00 00       	call   0x10215d30
10215a51:	89 44 24 5c          	mov    DWORD PTR [esp+0x5c],eax
10215a55:	8b 44 24 74          	mov    eax,DWORD PTR [esp+0x74]
10215a59:	83 c4 3c             	add    esp,0x3c
10215a5c:	85 c0                	test   eax,eax
10215a5e:	0f 84 26 fd ff ff    	je     0x1021578a
10215a64:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215a67:	8a 4c 24 14          	mov    cl,BYTE PTR [esp+0x14]
10215a6b:	53                   	push   ebx
10215a6c:	55                   	push   ebp
10215a6d:	50                   	push   eax
10215a6e:	8a 46 01             	mov    al,BYTE PTR [esi+0x1]
10215a71:	e9 32 02 00 00       	jmp    0x10215ca8
10215a76:	8a 56 02             	mov    dl,BYTE PTR [esi+0x2]
10215a79:	8a 44 24 1c          	mov    al,BYTE PTR [esp+0x1c]
10215a7d:	8a 4e 09             	mov    cl,BYTE PTR [esi+0x9]
10215a80:	52                   	push   edx
10215a81:	02 06                	add    al,BYTE PTR [esi]
10215a83:	50                   	push   eax
10215a84:	51                   	push   ecx
10215a85:	e8 a6 02 00 00       	call   0x10215d30
10215a8a:	8a 56 02             	mov    dl,BYTE PTR [esi+0x2]
10215a8d:	8a 4e 10             	mov    cl,BYTE PTR [esi+0x10]
10215a90:	53                   	push   ebx
10215a91:	55                   	push   ebp
10215a92:	8b f8                	mov    edi,eax
10215a94:	8a 44 24 30          	mov    al,BYTE PTR [esp+0x30]
10215a98:	52                   	push   edx
10215a99:	8a 16                	mov    dl,BYTE PTR [esi]
10215a9b:	02 c2                	add    al,dl
10215a9d:	50                   	push   eax
10215a9e:	51                   	push   ecx
10215a9f:	e8 8c 02 00 00       	call   0x10215d30
10215aa4:	8a 56 02             	mov    dl,BYTE PTR [esi+0x2]
10215aa7:	8a 4e 17             	mov    cl,BYTE PTR [esi+0x17]
10215aaa:	53                   	push   ebx
10215aab:	55                   	push   ebp
10215aac:	89 44 24 4c          	mov    DWORD PTR [esp+0x4c],eax
10215ab0:	8a 44 24 44          	mov    al,BYTE PTR [esp+0x44]
10215ab4:	52                   	push   edx
10215ab5:	8a 16                	mov    dl,BYTE PTR [esi]
10215ab7:	02 c2                	add    al,dl
10215ab9:	50                   	push   eax
10215aba:	51                   	push   ecx
10215abb:	e8 70 02 00 00       	call   0x10215d30
10215ac0:	89 44 24 5c          	mov    DWORD PTR [esp+0x5c],eax
10215ac4:	8b 44 24 74          	mov    eax,DWORD PTR [esp+0x74]
10215ac8:	83 c4 3c             	add    esp,0x3c
10215acb:	85 c0                	test   eax,eax
10215acd:	0f 84 b7 fc ff ff    	je     0x1021578a
10215ad3:	8a 44 24 14          	mov    al,BYTE PTR [esp+0x14]
10215ad7:	8a 0e                	mov    cl,BYTE PTR [esi]
10215ad9:	8a 56 02             	mov    dl,BYTE PTR [esi+0x2]
10215adc:	53                   	push   ebx
10215add:	55                   	push   ebp
10215ade:	02 c1                	add    al,cl
10215ae0:	52                   	push   edx
10215ae1:	50                   	push   eax
10215ae2:	e9 c4 01 00 00       	jmp    0x10215cab
10215ae7:	8b 4c 24 2c          	mov    ecx,DWORD PTR [esp+0x2c]
10215aeb:	8b 94 24 48 02 00 00 	mov    edx,DWORD PTR [esp+0x248]
10215af2:	db 44 24 18          	fild   DWORD PTR [esp+0x18]
10215af6:	8a 84 0c 18 01 00 00 	mov    al,BYTE PTR [esp+ecx*1+0x118]
10215afd:	3c 3c                	cmp    al,0x3c
10215aff:	8b 82 04 3d 00 00    	mov    eax,DWORD PTR [edx+0x3d04]
10215b05:	d8 58 60             	fcomp  DWORD PTR [eax+0x60]
10215b08:	df e0                	fnstsw ax
10215b0a:	0f 85 b2 01 00 00    	jne    0x10215cc2
10215b10:	f6 c4 41             	test   ah,0x41
10215b13:	8b 44 24 3c          	mov    eax,DWORD PTR [esp+0x3c]
10215b17:	0f 85 cc 00 00 00    	jne    0x10215be9
10215b1d:	85 c0                	test   eax,eax
10215b1f:	75 51                	jne    0x10215b72
10215b21:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215b24:	8a 54 24 14          	mov    dl,BYTE PTR [esp+0x14]
10215b28:	8a 46 04             	mov    al,BYTE PTR [esi+0x4]
10215b2b:	53                   	push   ebx
10215b2c:	55                   	push   ebp
10215b2d:	51                   	push   ecx
10215b2e:	8a 4e 01             	mov    cl,BYTE PTR [esi+0x1]
10215b31:	89 7c 24 48          	mov    DWORD PTR [esp+0x48],edi
10215b35:	02 d1                	add    dl,cl
10215b37:	52                   	push   edx
10215b38:	50                   	push   eax
10215b39:	e8 f2 01 00 00       	call   0x10215d30
10215b3e:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215b41:	8a 54 24 28          	mov    dl,BYTE PTR [esp+0x28]
10215b45:	53                   	push   ebx
10215b46:	55                   	push   ebp
10215b47:	51                   	push   ecx
10215b48:	8a 4e 01             	mov    cl,BYTE PTR [esi+0x1]
10215b4b:	8b f8                	mov    edi,eax
10215b4d:	8a 46 0b             	mov    al,BYTE PTR [esi+0xb]
10215b50:	02 d1                	add    dl,cl
10215b52:	52                   	push   edx
10215b53:	50                   	push   eax
10215b54:	e8 d7 01 00 00       	call   0x10215d30
10215b59:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215b5c:	8a 54 24 3c          	mov    dl,BYTE PTR [esp+0x3c]
10215b60:	53                   	push   ebx
10215b61:	55                   	push   ebp
10215b62:	51                   	push   ecx
10215b63:	8a 4e 01             	mov    cl,BYTE PTR [esi+0x1]
10215b66:	02 d1                	add    dl,cl
10215b68:	89 44 24 50          	mov    DWORD PTR [esp+0x50],eax
10215b6c:	8a 46 12             	mov    al,BYTE PTR [esi+0x12]
10215b6f:	52                   	push   edx
10215b70:	eb 49                	jmp    0x10215bbb
10215b72:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215b75:	8a 54 24 14          	mov    dl,BYTE PTR [esp+0x14]
10215b79:	8a 46 06             	mov    al,BYTE PTR [esi+0x6]
10215b7c:	53                   	push   ebx
10215b7d:	55                   	push   ebp
10215b7e:	51                   	push   ecx
10215b7f:	02 56 01             	add    dl,BYTE PTR [esi+0x1]
10215b82:	52                   	push   edx
10215b83:	50                   	push   eax
10215b84:	e8 a7 01 00 00       	call   0x10215d30
10215b89:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215b8c:	8a 54 24 28          	mov    dl,BYTE PTR [esp+0x28]
10215b90:	53                   	push   ebx
10215b91:	55                   	push   ebp
10215b92:	51                   	push   ecx
10215b93:	8a 4e 01             	mov    cl,BYTE PTR [esi+0x1]
10215b96:	8b f8                	mov    edi,eax
10215b98:	8a 46 0d             	mov    al,BYTE PTR [esi+0xd]
10215b9b:	02 d1                	add    dl,cl
10215b9d:	52                   	push   edx
10215b9e:	50                   	push   eax
10215b9f:	e8 8c 01 00 00       	call   0x10215d30
10215ba4:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215ba7:	8a 54 24 3c          	mov    dl,BYTE PTR [esp+0x3c]
10215bab:	53                   	push   ebx
10215bac:	55                   	push   ebp
10215bad:	51                   	push   ecx
10215bae:	8a 4e 01             	mov    cl,BYTE PTR [esi+0x1]
10215bb1:	02 d1                	add    dl,cl
10215bb3:	89 44 24 50          	mov    DWORD PTR [esp+0x50],eax
10215bb7:	8a 46 14             	mov    al,BYTE PTR [esi+0x14]
10215bba:	52                   	push   edx
10215bbb:	50                   	push   eax
10215bbc:	e8 6f 01 00 00       	call   0x10215d30
10215bc1:	89 44 24 5c          	mov    DWORD PTR [esp+0x5c],eax
10215bc5:	8b 44 24 74          	mov    eax,DWORD PTR [esp+0x74]
10215bc9:	83 c4 3c             	add    esp,0x3c
10215bcc:	85 c0                	test   eax,eax
10215bce:	0f 84 b6 fb ff ff    	je     0x1021578a
10215bd4:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215bd7:	8a 54 24 14          	mov    dl,BYTE PTR [esp+0x14]
10215bdb:	53                   	push   ebx
10215bdc:	55                   	push   ebp
10215bdd:	51                   	push   ecx
10215bde:	8a 4e 01             	mov    cl,BYTE PTR [esi+0x1]
10215be1:	02 d1                	add    dl,cl
10215be3:	52                   	push   edx
10215be4:	e9 c2 00 00 00       	jmp    0x10215cab
10215be9:	85 c0                	test   eax,eax
10215beb:	75 4e                	jne    0x10215c3b
10215bed:	8a 4c 24 14          	mov    cl,BYTE PTR [esp+0x14]
10215bf1:	8a 16                	mov    dl,BYTE PTR [esi]
10215bf3:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215bf6:	53                   	push   ebx
10215bf7:	55                   	push   ebp
10215bf8:	02 ca                	add    cl,dl
10215bfa:	8a 56 04             	mov    dl,BYTE PTR [esi+0x4]
10215bfd:	50                   	push   eax
10215bfe:	51                   	push   ecx
10215bff:	52                   	push   edx
10215c00:	89 7c 24 50          	mov    DWORD PTR [esp+0x50],edi
10215c04:	e8 27 01 00 00       	call   0x10215d30
10215c09:	8a 4c 24 28          	mov    cl,BYTE PTR [esp+0x28]
10215c0d:	8a 56 0b             	mov    dl,BYTE PTR [esi+0xb]
10215c10:	8b f8                	mov    edi,eax
10215c12:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215c15:	53                   	push   ebx
10215c16:	55                   	push   ebp
10215c17:	50                   	push   eax
10215c18:	8a 06                	mov    al,BYTE PTR [esi]
10215c1a:	02 c8                	add    cl,al
10215c1c:	51                   	push   ecx
10215c1d:	52                   	push   edx
10215c1e:	e8 0d 01 00 00       	call   0x10215d30
10215c23:	8a 4c 24 3c          	mov    cl,BYTE PTR [esp+0x3c]
10215c27:	89 44 24 44          	mov    DWORD PTR [esp+0x44],eax
10215c2b:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215c2e:	8a 56 12             	mov    dl,BYTE PTR [esi+0x12]
10215c31:	53                   	push   ebx
10215c32:	55                   	push   ebp
10215c33:	50                   	push   eax
10215c34:	8a 06                	mov    al,BYTE PTR [esi]
10215c36:	02 c8                	add    cl,al
10215c38:	51                   	push   ecx
10215c39:	eb 48                	jmp    0x10215c83
10215c3b:	8a 4c 24 14          	mov    cl,BYTE PTR [esp+0x14]
10215c3f:	8a 16                	mov    dl,BYTE PTR [esi]
10215c41:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215c44:	53                   	push   ebx
10215c45:	55                   	push   ebp
10215c46:	02 ca                	add    cl,dl
10215c48:	8a 56 06             	mov    dl,BYTE PTR [esi+0x6]
10215c4b:	50                   	push   eax
10215c4c:	51                   	push   ecx
10215c4d:	52                   	push   edx
10215c4e:	e8 dd 00 00 00       	call   0x10215d30
10215c53:	8a 4c 24 28          	mov    cl,BYTE PTR [esp+0x28]
10215c57:	8a 56 0d             	mov    dl,BYTE PTR [esi+0xd]
10215c5a:	8b f8                	mov    edi,eax
10215c5c:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215c5f:	53                   	push   ebx
10215c60:	55                   	push   ebp
10215c61:	50                   	push   eax
10215c62:	8a 06                	mov    al,BYTE PTR [esi]
10215c64:	02 c8                	add    cl,al
10215c66:	51                   	push   ecx
10215c67:	52                   	push   edx
10215c68:	e8 c3 00 00 00       	call   0x10215d30
10215c6d:	8a 4c 24 3c          	mov    cl,BYTE PTR [esp+0x3c]
10215c71:	89 44 24 44          	mov    DWORD PTR [esp+0x44],eax
10215c75:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215c78:	8a 56 14             	mov    dl,BYTE PTR [esi+0x14]
10215c7b:	53                   	push   ebx
10215c7c:	55                   	push   ebp
10215c7d:	50                   	push   eax
10215c7e:	8a 06                	mov    al,BYTE PTR [esi]
10215c80:	02 c8                	add    cl,al
10215c82:	51                   	push   ecx
10215c83:	52                   	push   edx
10215c84:	e8 a7 00 00 00       	call   0x10215d30
10215c89:	89 44 24 5c          	mov    DWORD PTR [esp+0x5c],eax
10215c8d:	8b 44 24 74          	mov    eax,DWORD PTR [esp+0x74]
10215c91:	83 c4 3c             	add    esp,0x3c
10215c94:	85 c0                	test   eax,eax
10215c96:	0f 84 ee fa ff ff    	je     0x1021578a
10215c9c:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215c9f:	8a 4c 24 14          	mov    cl,BYTE PTR [esp+0x14]
10215ca3:	53                   	push   ebx
10215ca4:	55                   	push   ebp
10215ca5:	50                   	push   eax
10215ca6:	8a 06                	mov    al,BYTE PTR [esi]
10215ca8:	02 c8                	add    cl,al
10215caa:	51                   	push   ecx
10215cab:	e8 e0 00 00 00       	call   0x10215d90
10215cb0:	8b 4c 24 30          	mov    ecx,DWORD PTR [esp+0x30]
10215cb4:	83 c4 10             	add    esp,0x10
10215cb7:	2b c8                	sub    ecx,eax
10215cb9:	89 4c 24 34          	mov    DWORD PTR [esp+0x34],ecx
10215cbd:	e9 c8 fa ff ff       	jmp    0x1021578a
10215cc2:	f6 c4 41             	test   ah,0x41
10215cc5:	53                   	push   ebx
10215cc6:	55                   	push   ebp
10215cc7:	75 10                	jne    0x10215cd9
10215cc9:	8a 56 01             	mov    dl,BYTE PTR [esi+0x1]
10215ccc:	8a 44 24 1c          	mov    al,BYTE PTR [esp+0x1c]
10215cd0:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215cd3:	02 d0                	add    dl,al
10215cd5:	51                   	push   ecx
10215cd6:	52                   	push   edx
10215cd7:	eb 0d                	jmp    0x10215ce6
10215cd9:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215cdc:	8a 4c 24 1c          	mov    cl,BYTE PTR [esp+0x1c]
10215ce0:	50                   	push   eax
10215ce1:	8a 06                	mov    al,BYTE PTR [esi]
10215ce3:	02 c8                	add    cl,al
10215ce5:	51                   	push   ecx
10215ce6:	e8 a5 00 00 00       	call   0x10215d90
10215ceb:	8b 54 24 44          	mov    edx,DWORD PTR [esp+0x44]
10215cef:	83 c4 10             	add    esp,0x10
10215cf2:	03 c2                	add    eax,edx
10215cf4:	50                   	push   eax
10215cf5:	8d 44 24 54          	lea    eax,[esp+0x54]
10215cf9:	68 5c cf 75 10       	push   0x1075cf5c
10215cfe:	50                   	push   eax
10215cff:	ff 15 54 a1 23 10    	call   DWORD PTR ds:0x1023a154
10215d05:	8b 4c 24 38          	mov    ecx,DWORD PTR [esp+0x38]
10215d09:	8d 94 24 24 01 00 00 	lea    edx,[esp+0x124]
10215d10:	51                   	push   ecx
10215d11:	8d 44 24 60          	lea    eax,[esp+0x60]
10215d15:	52                   	push   edx
10215d16:	50                   	push   eax
10215d17:	e8 84 8f f8 ff       	call   0x1019eca0
10215d1c:	83 c4 18             	add    esp,0x18
10215d1f:	e9           	jmp    0x10215846
