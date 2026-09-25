
/mnt/data/m27_work/msi_extract/flat/mtsyc32.dll:     file format pei-i386


Disassembly of section .text:

1019e4e0 <.text+0x19d4e0>:
1019e4e0:	75 04                	jne    0x1019e4e6
1019e4e2:	84 db                	test   bl,bl
1019e4e4:	74 28                	je     0x1019e50e
1019e4e6:	8a 45 00             	mov    al,BYTE PTR [ebp+0x0]
1019e4e9:	88 06                	mov    BYTE PTR [esi],al
1019e4eb:	8a 45 00             	mov    al,BYTE PTR [ebp+0x0]
1019e4ee:	3c a0                	cmp    al,0xa0
1019e4f0:	72 08                	jb     0x1019e4fa
1019e4f2:	3c af                	cmp    al,0xaf
1019e4f4:	77 04                	ja     0x1019e4fa
1019e4f6:	88 06                	mov    BYTE PTR [esi],al
1019e4f8:	eb 13                	jmp    0x1019e50d
1019e4fa:	3c e0                	cmp    al,0xe0
1019e4fc:	72 08                	jb     0x1019e506
1019e4fe:	3c ef                	cmp    al,0xef
1019e500:	77 04                	ja     0x1019e506
1019e502:	88 06                	mov    BYTE PTR [esi],al
1019e504:	eb 07                	jmp    0x1019e50d
1019e506:	3c f1                	cmp    al,0xf1
1019e508:	75 03                	jne    0x1019e50d
1019e50a:	c6 06 f1             	mov    BYTE PTR [esi],0xf1
1019e50d:	46                   	inc    esi
1019e50e:	8b 44 24 18          	mov    eax,DWORD PTR [esp+0x18]
1019e512:	45                   	inc    ebp
1019e513:	48                   	dec    eax
1019e514:	89 44 24 18          	mov    DWORD PTR [esp+0x18],eax
1019e518:	75 aa                	jne    0x1019e4c4
1019e51a:	5b                   	pop    ebx
1019e51b:	83 c9 ff             	or     ecx,0xffffffff
1019e51e:	33 c0                	xor    eax,eax
1019e520:	c6 06 00             	mov    BYTE PTR [esi],0x0
1019e523:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
1019e525:	f7 d1                	not    ecx
1019e527:	49                   	dec    ecx
1019e528:	5f                   	pop    edi
1019e529:	5e                   	pop    esi
1019e52a:	8b c1                	mov    eax,ecx
1019e52c:	5d                   	pop    ebp
1019e52d:	c3                   	ret
1019e52e:	90                   	nop
1019e52f:	90                   	nop
1019e530:	51                   	push   ecx
1019e531:	8b 44 24 0c          	mov    eax,DWORD PTR [esp+0xc]
1019e535:	8b 4c 24 08          	mov    ecx,DWORD PTR [esp+0x8]
1019e539:	56                   	push   esi
1019e53a:	8b 74 24 14          	mov    esi,DWORD PTR [esp+0x14]
1019e53e:	8b 91 6c 01 00 00    	mov    edx,DWORD PTR [ecx+0x16c]
1019e544:	8d 04 40             	lea    eax,[eax+eax*2]
1019e547:	8a 0e                	mov    cl,BYTE PTR [esi]
1019e549:	57                   	push   edi
1019e54a:	8b 04 82             	mov    eax,DWORD PTR [edx+eax*4]
1019e54d:	33 ff                	xor    edi,edi
1019e54f:	84 c9                	test   cl,cl
1019e551:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
1019e555:	0f 84 b8 00 00 00    	je     0x1019e613
1019e55b:	53                   	push   ebx
1019e55c:	83 c0 1c             	add    eax,0x1c
1019e55f:	55                   	push   ebp
1019e560:	8b 2d f8 a1 23 10    	mov    ebp,DWORD PTR ds:0x1023a1f8
1019e566:	c7 44 24 1c 04 00 00 	mov    DWORD PTR [esp+0x1c],0x4
1019e56d:	00 
1019e56e:	89 44 24 20          	mov    DWORD PTR [esp+0x20],eax
1019e572:	8a 06                	mov    al,BYTE PTR [esi]
1019e574:	50                   	push   eax
1019e575:	e8 86 35 07 00       	call   0x10211b00
1019e57a:	83 c4 04             	add    esp,0x4
1019e57d:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
1019e581:	85 c0                	test   eax,eax
1019e583:	74 61                	je     0x1019e5e6
1019e585:	80 7e 01 5b          	cmp    BYTE PTR [esi+0x1],0x5b
1019e589:	75 5b                	jne    0x1019e5e6
1019e58b:	8a 5e 02             	mov    bl,BYTE PTR [esi+0x2]
1019e58e:	83 c6 02             	add    esi,0x2
1019e591:	46                   	inc    esi
1019e592:	56                   	push   esi
1019e593:	ff d5                	call   ebp
1019e595:	8a 0e                	mov    cl,BYTE PTR [esi]
1019e597:	83 c4 04             	add    esp,0x4
1019e59a:	84 c9                	test   cl,cl
1019e59c:	74 0d                	je     0x1019e5ab
1019e59e:	80 f9 5d             	cmp    cl,0x5d
1019e5a1:	74 08                	je     0x1019e5ab
1019e5a3:	8a 4e 01             	mov    cl,BYTE PTR [esi+0x1]
1019e5a6:	46                   	inc    esi
1019e5a7:	84 c9                	test   cl,cl
1019e5a9:	75 f3                	jne    0x1019e59e
1019e5ab:	80 fb 65             	cmp    bl,0x65
1019e5ae:	74 26                	je     0x1019e5d6
1019e5b0:	80 fb 6c             	cmp    bl,0x6c
1019e5b3:	74 18                	je     0x1019e5cd
1019e5b5:	80 fb 74             	cmp    bl,0x74
1019e5b8:	75 22                	jne    0x1019e5dc
1019e5ba:	8b 54 24 1c          	mov    edx,DWORD PTR [esp+0x1c]
1019e5be:	0f be c8             	movsx  ecx,al
1019e5c1:	8b 44 24 18          	mov    eax,DWORD PTR [esp+0x18]
1019e5c5:	03 d7                	add    edx,edi
1019e5c7:	47                   	inc    edi
1019e5c8:	89 0c 90             	mov    DWORD PTR [eax+edx*4],ecx
1019e5cb:	eb 0f                	jmp    0x1019e5dc
1019e5cd:	8b 4c 24 20          	mov    ecx,DWORD PTR [esp+0x20]
1019e5d1:	88 41 01             	mov    BYTE PTR [ecx+0x1],al
1019e5d4:	eb 06                	jmp    0x1019e5dc
1019e5d6:	8b 54 24 20          	mov    edx,DWORD PTR [esp+0x20]
1019e5da:	88 02                	mov    BYTE PTR [edx],al
1019e5dc:	80 7e 01 5b          	cmp    BYTE PTR [esi+0x1],0x5b
1019e5e0:	74 a9                	je     0x1019e58b
1019e5e2:	8b 44 24 10          	mov    eax,DWORD PTR [esp+0x10]
1019e5e6:	80 3e 00             	cmp    BYTE PTR [esi],0x0
1019e5e9:	74 01                	je     0x1019e5ec
1019e5eb:	46                   	inc    esi
1019e5ec:	85 c0                	test   eax,eax
1019e5ee:	74 18                	je     0x1019e608
1019e5f0:	8b 4c 24 1c          	mov    ecx,DWORD PTR [esp+0x1c]
1019e5f4:	8b 44 24 20          	mov    eax,DWORD PTR [esp+0x20]
1019e5f8:	83 c1 08             	add    ecx,0x8
1019e5fb:	83 c0 20             	add    eax,0x20
1019e5fe:	89 4c 24 1c          	mov    DWORD PTR [esp+0x1c],ecx
1019e602:	89 44 24 20          	mov    DWORD PTR [esp+0x20],eax
1019e606:	33 ff                	xor    edi,edi
1019e608:	80 3e 00             	cmp    BYTE PTR [esi],0x0
1019e60b:	0f 85 61 ff ff ff    	jne    0x1019e572
1019e611:	5d                   	pop    ebp
1019e612:	5b                   	pop    ebx
1019e613:	5f                   	pop    edi
1019e614:	5e                   	pop    esi
1019e615:	59                   	pop    ecx
1019e616:	c3                   	ret
1019e617:	90                   	nop
1019e618:	90                   	nop
1019e619:	90                   	nop
1019e61a:	90                   	nop
1019e61b:	90                   	nop
1019e61c:	90                   	nop
1019e61d:	90                   	nop
1019e61e:	90                   	nop
1019e61f:	90                   	nop
1019e620:	81 ec 98 00 00 00    	sub    esp,0x98
1019e626:	53                   	push   ebx
1019e627:	55                   	push   ebp
1019e628:	56                   	push   esi
1019e629:	57                   	push   edi
1019e62a:	b9 18 00 00 00       	mov    ecx,0x18
1019e62f:	33 c0                	xor    eax,eax
1019e631:	8d 7c 24 45          	lea    edi,[esp+0x45]
1019e635:	c6 44 24 44 00       	mov    BYTE PTR [esp+0x44],0x0
1019e63a:	f3 ab                	rep stos DWORD PTR es:[edi],eax
1019e63c:	66 ab                	stos   WORD PTR es:[edi],ax
1019e63e:	68 00 01 00 00       	push   0x100
1019e643:	6a 01                	push   0x1
1019e645:	aa                   	stos   BYTE PTR es:[edi],al
1019e646:	e8 35 4c e6 ff       	call   0x10003280
1019e64b:	68 00 04 00 00       	push   0x400
1019e650:	6a 01                	push   0x1
1019e652:	89 44 24 40          	mov    DWORD PTR [esp+0x40],eax
1019e656:	c7 44 24 4c 00 04 00 	mov    DWORD PTR [esp+0x4c],0x400
1019e65d:	00 
1019e65e:	e8 1d 4c e6 ff       	call   0x10003280
1019e663:	8b bc 24 c4 00 00 00 	mov    edi,DWORD PTR [esp+0xc4]
1019e66a:	8b d0                	mov    edx,eax
1019e66c:	83 c9 ff             	or     ecx,0xffffffff
1019e66f:	33 c0                	xor    eax,eax
1019e671:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
1019e673:	f7 d1                	not    ecx
1019e675:	2b f9                	sub    edi,ecx
1019e677:	89 54 24 38          	mov    DWORD PTR [esp+0x38],edx
1019e67b:	8b c1                	mov    eax,ecx
1019e67d:	8b f7                	mov    esi,edi
1019e67f:	8b fa                	mov    edi,edx
1019e681:	89 54 24 3c          	mov    DWORD PTR [esp+0x3c],edx
1019e685:	c1 e9 02             	shr    ecx,0x2
1019e688:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
1019e68a:	8b 9c 24 bc 00 00 00 	mov    ebx,DWORD PTR [esp+0xbc]
1019e691:	8b c8                	mov    ecx,eax
1019e693:	83 e1 03             	and    ecx,0x3
1019e696:	f3 a4                	rep movs BYTE PTR es:[edi],BYTE PTR ds:[esi]
1019e698:	8d 4c 24 38          	lea    ecx,[esp+0x38]
1019e69c:	51                   	push   ecx
1019e69d:	53                   	push   ebx
1019e69e:	e8 8d 32 07 00       	call   0x10211930
1019e6a3:	8b 84 24 c8 00 00 00 	mov    eax,DWORD PTR [esp+0xc8]
1019e6aa:	33 f6                	xor    esi,esi
1019e6ac:	56                   	push   esi
1019e6ad:	56                   	push   esi
1019e6ae:	8d 14 40             	lea    edx,[eax+eax*2]
1019e6b1:	8b 83 6c 01 00 00    	mov    eax,DWORD PTR [ebx+0x16c]
1019e6b7:	53                   	push   ebx
1019e6b8:	89 74 24 38          	mov    DWORD PTR [esp+0x38],esi
1019e6bc:	8b 2c 90             	mov    ebp,DWORD PTR [eax+edx*4]
1019e6bf:	8d 3c 90             	lea    edi,[eax+edx*4]
1019e6c2:	89 74 24 34          	mov    DWORD PTR [esp+0x34],esi
1019e6c6:	89 7c 24 3c          	mov    DWORD PTR [esp+0x3c],edi
1019e6ca:	89 6c 24 48          	mov    DWORD PTR [esp+0x48],ebp
1019e6ce:	e8 3d 32 07 00       	call   0x10211910
1019e6d3:	53                   	push   ebx
1019e6d4:	e8 a7 32 07 00       	call   0x10211980
1019e6d9:	53                   	push   ebx
1019e6da:	e8 81 32 07 00       	call   0x10211960
1019e6df:	83 c4 2c             	add    esp,0x2c
1019e6e2:	83 f8 01             	cmp    eax,0x1
1019e6e5:	0f 84 83 01 00 00    	je     0x1019e86e
1019e6eb:	45                   	inc    ebp
1019e6ec:	53                   	push   ebx
1019e6ed:	e8 5e 32 07 00       	call   0x10211950
1019e6f2:	8b f8                	mov    edi,eax
1019e6f4:	83 c9 ff             	or     ecx,0xffffffff
1019e6f7:	33 c0                	xor    eax,eax
1019e6f9:	8d 54 24 48          	lea    edx,[esp+0x48]
1019e6fd:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
1019e6ff:	f7 d1                	not    ecx
1019e701:	2b f9                	sub    edi,ecx
1019e703:	53                   	push   ebx
1019e704:	8b c1                	mov    eax,ecx
1019e706:	8b f7                	mov    esi,edi
1019e708:	8b fa                	mov    edi,edx
1019e70a:	c1 e9 02             	shr    ecx,0x2
1019e70d:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
1019e70f:	8b c8                	mov    ecx,eax
1019e711:	83 e1 03             	and    ecx,0x3
1019e714:	f3 a4                	rep movs BYTE PTR es:[edi],BYTE PTR ds:[esi]
1019e716:	8a 4c 24 4c          	mov    cl,BYTE PTR [esp+0x4c]
1019e71a:	88 4d ff             	mov    BYTE PTR [ebp-0x1],cl
1019e71d:	e8 3e 32 07 00       	call   0x10211960
1019e722:	8b 54 24 18          	mov    edx,DWORD PTR [esp+0x18]
1019e726:	88 45 02             	mov    BYTE PTR [ebp+0x2],al
1019e729:	8b 84 24 b8 00 00 00 	mov    eax,DWORD PTR [esp+0xb8]
1019e730:	89 55 07             	mov    DWORD PTR [ebp+0x7],edx
1019e733:	89 45 03             	mov    DWORD PTR [ebp+0x3],eax
1019e736:	53                   	push   ebx
1019e737:	c6 45 00 00          	mov    BYTE PTR [ebp+0x0],0x0
1019e73b:	e8 30 32 07 00       	call   0x10211970
1019e740:	83 c4 0c             	add    esp,0xc
1019e743:	a8 08                	test   al,0x8
1019e745:	74 5f                	je     0x1019e7a6
1019e747:	8d 4c 24 1c          	lea    ecx,[esp+0x1c]
1019e74b:	8d 54 24 20          	lea    edx,[esp+0x20]
1019e74f:	51                   	push   ecx
1019e750:	52                   	push   edx
1019e751:	53                   	push   ebx
1019e752:	e8 99 31 07 00       	call   0x102118f0
1019e757:	8b 44 24 2c          	mov    eax,DWORD PTR [esp+0x2c]
1019e75b:	83 c4 0c             	add    esp,0xc
1019e75e:	85 c0                	test   eax,eax
1019e760:	74 1d                	je     0x1019e77f
1019e762:	8a 45 00             	mov    al,BYTE PTR [ebp+0x0]
1019e765:	0c 04                	or     al,0x4
1019e767:	88 45 00             	mov    BYTE PTR [ebp+0x0],al
1019e76a:	8b 44 24 18          	mov    eax,DWORD PTR [esp+0x18]
1019e76e:	8b 4c 24 20          	mov    ecx,DWORD PTR [esp+0x20]
1019e772:	8b 50 08             	mov    edx,DWORD PTR [eax+0x8]
1019e775:	83 e1 0f             	and    ecx,0xf
1019e778:	0b d1                	or     edx,ecx
1019e77a:	89 50 08             	mov    DWORD PTR [eax+0x8],edx
1019e77d:	eb 04                	jmp    0x1019e783
1019e77f:	8b 44 24 18          	mov    eax,DWORD PTR [esp+0x18]
1019e783:	8b 4c 24 1c          	mov    ecx,DWORD PTR [esp+0x1c]
1019e787:	85 c9                	test   ecx,ecx
1019e789:	74 1b                	je     0x1019e7a6
1019e78b:	8a 4d 01             	mov    cl,BYTE PTR [ebp+0x1]
1019e78e:	80 c9 01             	or     cl,0x1
1019e791:	88 4d 01             	mov    BYTE PTR [ebp+0x1],cl
1019e794:	8b 54 24 1c          	mov    edx,DWORD PTR [esp+0x1c]
1019e798:	8b 48 08             	mov    ecx,DWORD PTR [eax+0x8]
1019e79b:	83 e2 0f             	and    edx,0xf
1019e79e:	c1 e2 04             	shl    edx,0x4
1019e7a1:	0b ca                	or     ecx,edx
1019e7a3:	89 48 08             	mov    DWORD PTR [eax+0x8],ecx
1019e7a6:	53                   	push   ebx
1019e7a7:	e8 c4 31 07 00       	call   0x10211970
1019e7ac:	83 c4 04             	add    esp,0x4
1019e7af:	a8 02                	test   al,0x2
1019e7b1:	74 04                	je     0x1019e7b7
1019e7b3:	80 4d 00 01          	or     BYTE PTR [ebp+0x0],0x1
1019e7b7:	53                   	push   ebx
1019e7b8:	e8 b3 31 07 00       	call   0x10211970
1019e7bd:	83 c4 04             	add    esp,0x4
1019e7c0:	a8 04                	test   al,0x4
1019e7c2:	74 04                	je     0x1019e7c8
1019e7c4:	80 4d 00 02          	or     BYTE PTR [ebp+0x0],0x2
1019e7c8:	0f be 74 24 44       	movsx  esi,BYTE PTR [esp+0x44]
1019e7cd:	8b 3d 1c a2 23 10    	mov    edi,DWORD PTR ds:0x1023a21c
1019e7d3:	56                   	push   esi
1019e7d4:	68 74 8a 4e 10       	push   0x104e8a74
1019e7d9:	ff d7                	call   edi
1019e7db:	83 c4 08             	add    esp,0x8
1019e7de:	85 c0                	test   eax,eax
1019e7e0:	74 04                	je     0x1019e7e6
1019e7e2:	80 4d 00 08          	or     BYTE PTR [ebp+0x0],0x8
1019e7e6:	56                   	push   esi
1019e7e7:	68 7c 8a 4e 10       	push   0x104e8a7c
1019e7ec:	ff d7                	call   edi
1019e7ee:	83 c4 08             	add    esp,0x8
1019e7f1:	85 c0                	test   eax,eax
1019e7f3:	74 04                	je     0x1019e7f9
1019e7f5:	80 4d 00 20          	or     BYTE PTR [ebp+0x0],0x20
1019e7f9:	56                   	push   esi
1019e7fa:	68 60 8a 4e 10       	push   0x104e8a60
1019e7ff:	ff d7                	call   edi
1019e801:	83 c4 08             	add    esp,0x8
1019e804:	85 c0                	test   eax,eax
1019e806:	74 04                	je     0x1019e80c
1019e808:	80 4d 00 10          	or     BYTE PTR [ebp+0x0],0x10
1019e80c:	56                   	push   esi
1019e80d:	68 6c 8a 4e 10       	push   0x104e8a6c
1019e812:	ff d7                	call   edi
1019e814:	83 c4 08             	add    esp,0x8
1019e817:	85 c0                	test   eax,eax
1019e819:	74 04                	je     0x1019e81f
1019e81b:	80 4d 00 40          	or     BYTE PTR [ebp+0x0],0x40
1019e81f:	53                   	push   ebx
1019e820:	e8 3b 31 07 00       	call   0x10211960
1019e825:	83 c4 04             	add    esp,0x4
1019e828:	a8 02                	test   al,0x2
1019e82a:	8b 44 24 10          	mov    eax,DWORD PTR [esp+0x10]
1019e82e:	74 0a                	je     0x1019e83a
1019e830:	85 c0                	test   eax,eax
1019e832:	74 0a                	je     0x1019e83e
1019e834:	80 4d e0 08          	or     BYTE PTR [ebp-0x20],0x8
1019e838:	eb 04                	jmp    0x1019e83e
1019e83a:	89 44 24 14          	mov    DWORD PTR [esp+0x14],eax
1019e83e:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
1019e842:	53                   	push   ebx
1019e843:	42                   	inc    edx
1019e844:	83 c5 20             	add    ebp,0x20
1019e847:	89 54 24 14          	mov    DWORD PTR [esp+0x14],edx
1019e84b:	e8 30 31 07 00       	call   0x10211980
