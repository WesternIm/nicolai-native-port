
/mnt/data/m27_work/msi_extract/flat/mtsyc32.dll:     file format pei-i386


Disassembly of section .text:

10217220 <.text+0x216220>:
10217220:	00 3c 75 14 8b 85 28 	add    BYTE PTR [esi*2+0x28858b14],bh
10217227:	01 00                	add    DWORD PTR [eax],eax
10217229:	00 8b 5c 24 10 03    	add    BYTE PTR [ebx+0x310245c],cl
1021722f:	c3                   	ret
10217230:	80 38 20             	cmp    BYTE PTR [eax],0x20
10217233:	75 03                	jne    0x10217238
10217235:	c6 00 5f             	mov    BYTE PTR [eax],0x5f
10217238:	8b 85 24 01 00 00    	mov    eax,DWORD PTR [ebp+0x124]
1021723e:	33 db                	xor    ebx,ebx
10217240:	46                   	inc    esi
10217241:	83 c7 14             	add    edi,0x14
10217244:	8a 1c 01             	mov    bl,BYTE PTR [ecx+eax*1]
10217247:	3b f3                	cmp    esi,ebx
10217249:	7e c3                	jle    0x1021720e
1021724b:	e9 f1 00 00 00       	jmp    0x10217341
10217250:	8b 8d 3c 01 00 00    	mov    ecx,DWORD PTR [ebp+0x13c]
10217256:	8b 44 24 10          	mov    eax,DWORD PTR [esp+0x10]
1021725a:	8d 54 24 24          	lea    edx,[esp+0x24]
1021725e:	8b 7c 81 04          	mov    edi,DWORD PTR [ecx+eax*4+0x4]
10217262:	83 c9 ff             	or     ecx,0xffffffff
10217265:	33 c0                	xor    eax,eax
10217267:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
10217269:	f7 d1                	not    ecx
1021726b:	2b f9                	sub    edi,ecx
1021726d:	8b c1                	mov    eax,ecx
1021726f:	8b f7                	mov    esi,edi
10217271:	8b fa                	mov    edi,edx
10217273:	8d 94 24 88 00 00 00 	lea    edx,[esp+0x88]
1021727a:	c1 e9 02             	shr    ecx,0x2
1021727d:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
1021727f:	8b c8                	mov    ecx,eax
10217281:	83 e1 03             	and    ecx,0x3
10217284:	f3 a4                	rep movs BYTE PTR es:[edi],BYTE PTR ds:[esi]
10217286:	8d 4c 24 24          	lea    ecx,[esp+0x24]
1021728a:	51                   	push   ecx
1021728b:	68 0c 03 73 10       	push   0x1073030c
10217290:	52                   	push   edx
10217291:	ff 15 54 a1 23 10    	call   DWORD PTR ds:0x1023a154
10217297:	8d 84 24 94 00 00 00 	lea    eax,[esp+0x94]
1021729e:	50                   	push   eax
1021729f:	68 78 d0 75 10       	push   0x1075d078
102172a4:	e8 77 76 f8 ff       	call   0x1019e920
102172a9:	83 c4 14             	add    esp,0x14
102172ac:	85 c0                	test   eax,eax
102172ae:	0f 85 8d 00 00 00    	jne    0x10217341
102172b4:	8b 8d 28 01 00 00    	mov    ecx,DWORD PTR [ebp+0x128]
102172ba:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
102172be:	8d 04 0a             	lea    eax,[edx+ecx*1]
102172c1:	8a 0c 0a             	mov    cl,BYTE PTR [edx+ecx*1]
102172c4:	80 f9 20             	cmp    cl,0x20
102172c7:	75 78                	jne    0x10217341
102172c9:	c6 00 3d             	mov    BYTE PTR [eax],0x3d
102172cc:	eb 73                	jmp    0x10217341
102172ce:	8b 4c 24 10          	mov    ecx,DWORD PTR [esp+0x10]
102172d2:	bb 01 00 00 00       	mov    ebx,0x1
102172d7:	41                   	inc    ecx
102172d8:	8d 14 89             	lea    edx,[ecx+ecx*4]
102172db:	8d 34 d2             	lea    esi,[edx+edx*8]
102172de:	c1 e6 03             	shl    esi,0x3
102172e1:	2b f1                	sub    esi,ecx
102172e3:	c1 e6 02             	shl    esi,0x2
102172e6:	38 1c 06             	cmp    BYTE PTR [esi+eax*1],bl
102172e9:	72 56                	jb     0x10217341
102172eb:	bf 14 00 00 00       	mov    edi,0x14
102172f0:	8b 54 24 1c          	mov    edx,DWORD PTR [esp+0x1c]
102172f4:	6a ff                	push   0xffffffff
102172f6:	6a 0a                	push   0xa
102172f8:	6a 07                	push   0x7
102172fa:	03 c2                	add    eax,edx
102172fc:	6a 06                	push   0x6
102172fe:	33 c9                	xor    ecx,ecx
10217300:	6a 05                	push   0x5
10217302:	8a 8c 38 a3 05 00 00 	mov    cl,BYTE PTR [eax+edi*1+0x5a3]
10217309:	6a 04                	push   0x4
1021730b:	6a 03                	push   0x3
1021730d:	51                   	push   ecx
1021730e:	e8 6d b3 ff ff       	call   0x10212680
10217313:	83 c4 20             	add    esp,0x20
10217316:	84 c0                	test   al,al
10217318:	74 14                	je     0x1021732e
1021731a:	8b 95 28 01 00 00    	mov    edx,DWORD PTR [ebp+0x128]
10217320:	8b 44 24 10          	mov    eax,DWORD PTR [esp+0x10]
10217324:	03 c2                	add    eax,edx
10217326:	80 38 20             	cmp    BYTE PTR [eax],0x20
10217329:	75 03                	jne    0x1021732e
1021732b:	c6 00 5f             	mov    BYTE PTR [eax],0x5f
1021732e:	8b 85 24 01 00 00    	mov    eax,DWORD PTR [ebp+0x124]
10217334:	33 c9                	xor    ecx,ecx
10217336:	43                   	inc    ebx
10217337:	83 c7 14             	add    edi,0x14
1021733a:	8a 0c 06             	mov    cl,BYTE PTR [esi+eax*1]
1021733d:	3b d9                	cmp    ebx,ecx
1021733f:	7e af                	jle    0x102172f0
10217341:	8b 85 24 01 00 00    	mov    eax,DWORD PTR [ebp+0x124]
10217347:	8b 54 24 1c          	mov    edx,DWORD PTR [esp+0x1c]
1021734b:	8b 74 24 20          	mov    esi,DWORD PTR [esp+0x20]
1021734f:	8b 5c 24 18          	mov    ebx,DWORD PTR [esp+0x18]
10217353:	8d 0c 10             	lea    ecx,[eax+edx*1]
10217356:	33 d2                	xor    edx,edx
10217358:	46                   	inc    esi
10217359:	83 c3 14             	add    ebx,0x14
1021735c:	8a 11                	mov    dl,BYTE PTR [ecx]
1021735e:	89 74 24 20          	mov    DWORD PTR [esp+0x20],esi
10217362:	3b f2                	cmp    esi,edx
10217364:	89 5c 24 18          	mov    DWORD PTR [esp+0x18],ebx
10217368:	0f 8e 70 fb ff ff    	jle    0x10216ede
1021736e:	8b 5c 24 10          	mov    ebx,DWORD PTR [esp+0x10]
10217372:	eb 0a                	jmp    0x1021737e
10217374:	8b 85 28 01 00 00    	mov    eax,DWORD PTR [ebp+0x128]
1021737a:	c6 04 03 2b          	mov    BYTE PTR [ebx+eax*1],0x2b
1021737e:	8b 8d 34 01 00 00    	mov    ecx,DWORD PTR [ebp+0x134]
10217384:	43                   	inc    ebx
10217385:	49                   	dec    ecx
10217386:	89 5c 24 10          	mov    DWORD PTR [esp+0x10],ebx
1021738a:	3b d9                	cmp    ebx,ecx
1021738c:	0f 8e 34 f9 ff ff    	jle    0x10216cc6
10217392:	8b 95 34 01 00 00    	mov    edx,DWORD PTR [ebp+0x134]
10217398:	8b 85 2c 01 00 00    	mov    eax,DWORD PTR [ebp+0x12c]
1021739e:	8b 0d 74 d0 75 10    	mov    ecx,DWORD PTR ds:0x1075d074
102173a4:	89 0c 90             	mov    DWORD PTR [eax+edx*4],ecx
102173a7:	8b 95 34 01 00 00    	mov    edx,DWORD PTR [ebp+0x134]
102173ad:	b8 01 00 00 00       	mov    eax,0x1
102173b2:	4a                   	dec    edx
102173b3:	3b d0                	cmp    edx,eax
102173b5:	7c 45                	jl     0x102173fc
102173b7:	8b 8d 28 01 00 00    	mov    ecx,DWORD PTR [ebp+0x128]
102173bd:	80 3c 08 2c          	cmp    BYTE PTR [eax+ecx*1],0x2c
102173c1:	75 1e                	jne    0x102173e1
102173c3:	8b 95 2c 01 00 00    	mov    edx,DWORD PTR [ebp+0x12c]
102173c9:	8d 0c 82             	lea    ecx,[edx+eax*4]
102173cc:	66 8b 15 b0 a8 6d 10 	mov    dx,WORD PTR ds:0x106da8b0
102173d3:	66 89 11             	mov    WORD PTR [ecx],dx
102173d6:	8a 15 b2 a8 6d 10    	mov    dl,BYTE PTR ds:0x106da8b2
102173dc:	88 51 02             	mov    BYTE PTR [ecx+0x2],dl
102173df:	eb 0f                	jmp    0x102173f0
102173e1:	8b 8d 2c 01 00 00    	mov    ecx,DWORD PTR [ebp+0x12c]
102173e7:	8a 15 e4 0f 76 10    	mov    dl,BYTE PTR ds:0x10760fe4
102173ed:	88 14 81             	mov    BYTE PTR [ecx+eax*4],dl
102173f0:	8b 8d 34 01 00 00    	mov    ecx,DWORD PTR [ebp+0x134]
102173f6:	40                   	inc    eax
102173f7:	49                   	dec    ecx
102173f8:	3b c1                	cmp    eax,ecx
102173fa:	7e bb                	jle    0x102173b7
102173fc:	8b 85 34 01 00 00    	mov    eax,DWORD PTR [ebp+0x134]
10217402:	bb 01 00 00 00       	mov    ebx,0x1
10217407:	8d 50 ff             	lea    edx,[eax-0x1]
1021740a:	3b d3                	cmp    edx,ebx
1021740c:	0f 8c 4d 01 00 00    	jl     0x1021755f
10217412:	be 02 00 00 00       	mov    esi,0x2
10217417:	c7 44 24 1c 38 0b 00 	mov    DWORD PTR [esp+0x1c],0xb38
1021741e:	00 
1021741f:	89 74 24 14          	mov    DWORD PTR [esp+0x14],esi
10217423:	eb 04                	jmp    0x10217429
10217425:	8b 74 24 14          	mov    esi,DWORD PTR [esp+0x14]
10217429:	3b f0                	cmp    esi,eax
1021742b:	89 74 24 18          	mov    DWORD PTR [esp+0x18],esi
1021742f:	7d 48                	jge    0x10217479
10217431:	8b 7c 24 1c          	mov    edi,DWORD PTR [esp+0x1c]
10217435:	8b 85 2c 01 00 00    	mov    eax,DWORD PTR [ebp+0x12c]
1021743b:	80 3c b0 00          	cmp    BYTE PTR [eax+esi*4],0x0
1021743f:	75 38                	jne    0x10217479
10217441:	8b 8d 3c 01 00 00    	mov    ecx,DWORD PTR [ebp+0x13c]
10217447:	46                   	inc    esi
10217448:	6a 3c                	push   0x3c
1021744a:	81 c7 9c 05 00 00    	add    edi,0x59c
10217450:	8b 14 b1             	mov    edx,DWORD PTR [ecx+esi*4]
10217453:	52                   	push   edx
10217454:	ff 15 1c a2 23 10    	call   DWORD PTR ds:0x1023a21c
1021745a:	83 c4 08             	add    esp,0x8
1021745d:	85 c0                	test   eax,eax
1021745f:	75 0c                	jne    0x1021746d
10217461:	8b 85 24 01 00 00    	mov    eax,DWORD PTR [ebp+0x124]
10217467:	80 3c 38 00          	cmp    BYTE PTR [eax+edi*1],0x0
1021746b:	75 04                	jne    0x10217471
1021746d:	ff 44 24 18          	inc    DWORD PTR [esp+0x18]
10217471:	3b b5 34 01 00 00    	cmp    esi,DWORD PTR [ebp+0x134]
10217477:	7c bc                	jl     0x10217435
10217479:	8b 44 24 18          	mov    eax,DWORD PTR [esp+0x18]
1021747d:	2b c3                	sub    eax,ebx
1021747f:	83 f8 05             	cmp    eax,0x5
10217482:	0f 8c ae 00 00 00    	jl     0x10217536
10217488:	8d 04 1e             	lea    eax,[esi+ebx*1]
1021748b:	99                   	cdq
1021748c:	2b c2                	sub    eax,edx
1021748e:	33 d2                	xor    edx,edx
10217490:	d1 f8                	sar    eax,1
10217492:	3b c3                	cmp    eax,ebx
10217494:	89 54 24 10          	mov    DWORD PTR [esp+0x10],edx
10217498:	8b c8                	mov    ecx,eax
1021749a:	7c 2e                	jl     0x102174ca
1021749c:	8b 95 2c 01 00 00    	mov    edx,DWORD PTR [ebp+0x12c]
102174a2:	8d 14 82             	lea    edx,[edx+eax*4]
102174a5:	80 3a 00             	cmp    BYTE PTR [edx],0x0
102174a8:	75 0c                	jne    0x102174b6
102174aa:	8b bd 28 01 00 00    	mov    edi,DWORD PTR [ebp+0x128]
102174b0:	80 3c 39 20          	cmp    BYTE PTR [ecx+edi*1],0x20
102174b4:	74 0e                	je     0x102174c4
102174b6:	49                   	dec    ecx
102174b7:	83 ea 04             	sub    edx,0x4
102174ba:	3b cb                	cmp    ecx,ebx
102174bc:	7d e7                	jge    0x102174a5
102174be:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
102174c2:	eb 06                	jmp    0x102174ca
102174c4:	8b d1                	mov    edx,ecx
102174c6:	89 54 24 10          	mov    DWORD PTR [esp+0x10],edx
102174ca:	3b c6                	cmp    eax,esi
102174cc:	8b c8                	mov    ecx,eax
102174ce:	7f 3a                	jg     0x1021750a
102174d0:	8b 95 2c 01 00 00    	mov    edx,DWORD PTR [ebp+0x12c]
102174d6:	8d 14 82             	lea    edx,[edx+eax*4]
102174d9:	80 3a 00             	cmp    BYTE PTR [edx],0x0
102174dc:	75 0c                	jne    0x102174ea
102174de:	8b bd 28 01 00 00    	mov    edi,DWORD PTR [ebp+0x128]
102174e4:	80 3c 39 20          	cmp    BYTE PTR [ecx+edi*1],0x20
102174e8:	74 0e                	je     0x102174f8
102174ea:	41                   	inc    ecx
102174eb:	83 c2 04             	add    edx,0x4
102174ee:	3b ce                	cmp    ecx,esi
102174f0:	7e e7                	jle    0x102174d9
102174f2:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
102174f6:	eb 12                	jmp    0x1021750a
102174f8:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
102174fc:	8b f0                	mov    esi,eax
102174fe:	8b f9                	mov    edi,ecx
10217500:	2b f2                	sub    esi,edx
10217502:	2b f8                	sub    edi,eax
10217504:	3b f7                	cmp    esi,edi
10217506:	7e 02                	jle    0x1021750a
10217508:	8b d1                	mov    edx,ecx
1021750a:	85 d2                	test   edx,edx
1021750c:	7e 28                	jle    0x10217536
1021750e:	8b 85 2c 01 00 00    	mov    eax,DWORD PTR [ebp+0x12c]
10217514:	66 8b 0d 00 36 6f 10 	mov    cx,WORD PTR ds:0x106f3600
1021751b:	4b                   	dec    ebx
1021751c:	66 89 0c 90          	mov    WORD PTR [eax+edx*4],cx
10217520:	8b 4c 24 14          	mov    ecx,DWORD PTR [esp+0x14]
10217524:	8b 44 24 1c          	mov    eax,DWORD PTR [esp+0x1c]
10217528:	49                   	dec    ecx
10217529:	2d 9c 05 00 00       	sub    eax,0x59c
1021752e:	89 4c 24 14          	mov    DWORD PTR [esp+0x14],ecx
10217532:	89 44 24 1c          	mov    DWORD PTR [esp+0x1c],eax
10217536:	8b 54 24 1c          	mov    edx,DWORD PTR [esp+0x1c]
1021753a:	8b 85 34 01 00 00    	mov    eax,DWORD PTR [ebp+0x134]
10217540:	8b 74 24 14          	mov    esi,DWORD PTR [esp+0x14]
10217544:	81 c2 9c 05 00 00    	add    edx,0x59c
1021754a:	89 54 24 1c          	mov    DWORD PTR [esp+0x1c],edx
1021754e:	43                   	inc    ebx
1021754f:	8d 50 ff             	lea    edx,[eax-0x1]
10217552:	46                   	inc    esi
10217553:	3b da                	cmp    ebx,edx
10217555:	89 74 24 14          	mov    DWORD PTR [esp+0x14],esi
10217559:	0f 8e c6 fe ff ff    	jle    0x10217425
1021755f:	8b 85 34 01 00 00    	mov    eax,DWORD PTR [ebp+0x134]
10217565:	bb 01 00 00 00       	mov    ebx,0x1
1021756a:	8d 48 ff             	lea    ecx,[eax-0x1]
1021756d:	3b cb                	cmp    ecx,ebx
1021756f:	0f 8c 2c 01 00 00    	jl     0x102176a1
10217575:	c7 44 24 14 9c 05 00 	mov    DWORD PTR [esp+0x14],0x59c
1021757c:	00 
1021757d:	3b d8                	cmp    ebx,eax
1021757f:	8b f3                	mov    esi,ebx
10217581:	89 5c 24 18          	mov    DWORD PTR [esp+0x18],ebx
10217585:	7d 48                	jge    0x102175cf
10217587:	8b 7c 24 14          	mov    edi,DWORD PTR [esp+0x14]
1021758b:	8b 95 2c 01 00 00    	mov    edx,DWORD PTR [ebp+0x12c]
10217591:	80 3c b2 00          	cmp    BYTE PTR [edx+esi*4],0x0
10217595:	75 38                	jne    0x102175cf
10217597:	8b 85 3c 01 00 00    	mov    eax,DWORD PTR [ebp+0x13c]
1021759d:	46                   	inc    esi
1021759e:	6a 3c                	push   0x3c
102175a0:	81 c7 9c 05 00 00    	add    edi,0x59c
102175a6:	8b 0c b0             	mov    ecx,DWORD PTR [eax+esi*4]
102175a9:	51                   	push   ecx
102175aa:	ff 15 1c a2 23 10    	call   DWORD PTR ds:0x1023a21c
102175b0:	83 c4 08             	add    esp,0x8
102175b3:	85 c0                	test   eax,eax
102175b5:	75 0c                	jne    0x102175c3
102175b7:	8b 95 24 01 00 00    	mov    edx,DWORD PTR [ebp+0x124]
102175bd:	80 3c 3a 00          	cmp    BYTE PTR [edx+edi*1],0x0
102175c1:	75 04                	jne    0x102175c7
102175c3:	ff 44 24 18          	inc    DWORD PTR [esp+0x18]
102175c7:	3b b5 34 01 00 00    	cmp    esi,DWORD PTR [ebp+0x134]
102175cd:	7c bc                	jl     0x1021758b
102175cf:	8b 44 24 18          	mov    eax,DWORD PTR [esp+0x18]
102175d3:	2b c3                	sub    eax,ebx
102175d5:	83 f8 06             	cmp    eax,0x6
102175d8:	0f 8e a3 00 00 00    	jle    0x10217681
102175de:	8d 04 1e             	lea    eax,[esi+ebx*1]
102175e1:	99                   	cdq
102175e2:	2b c2                	sub    eax,edx
102175e4:	33 d2                	xor    edx,edx
102175e6:	d1 f8                	sar    eax,1
102175e8:	3b c3                	cmp    eax,ebx
102175ea:	89 54 24 10          	mov    DWORD PTR [esp+0x10],edx
102175ee:	8b c8                	mov    ecx,eax
102175f0:	7c 2e                	jl     0x10217620
102175f2:	8b 95 2c 01 00 00    	mov    edx,DWORD PTR [ebp+0x12c]
102175f8:	8d 14 82             	lea    edx,[edx+eax*4]
102175fb:	80 3a 00             	cmp    BYTE PTR [edx],0x0
102175fe:	75 0c                	jne    0x1021760c
10217600:	8b bd 28 01 00 00    	mov    edi,DWORD PTR [ebp+0x128]
10217606:	80 3c 39 5f          	cmp    BYTE PTR [ecx+edi*1],0x5f
1021760a:	74 0e                	je     0x1021761a
1021760c:	49                   	dec    ecx
1021760d:	83 ea 04             	sub    edx,0x4
10217610:	3b cb                	cmp    ecx,ebx
10217612:	7d e7                	jge    0x102175fb
10217614:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
10217618:	eb 06                	jmp    0x10217620
1021761a:	8b d1                	mov    edx,ecx
1021761c:	89 54 24 10          	mov    DWORD PTR [esp+0x10],edx
10217620:	3b c6                	cmp    eax,esi
10217622:	8b c8                	mov    ecx,eax
10217624:	7f 3a                	jg     0x10217660
10217626:	8b 95 2c 01 00 00    	mov    edx,DWORD PTR [ebp+0x12c]
1021762c:	8d 14 82             	lea    edx,[edx+eax*4]
1021762f:	80 3a 00             	cmp    BYTE PTR [edx],0x0
10217632:	75 0c                	jne    0x10217640
10217634:	8b bd 28 01 00 00    	mov    edi,DWORD PTR [ebp+0x128]
1021763a:	80 3c 39 5f          	cmp    BYTE PTR [ecx+edi*1],0x5f
1021763e:	74 0e                	je     0x1021764e
10217640:	41                   	inc    ecx
10217641:	83 c2 04             	add    edx,0x4
10217644:	3b ce                	cmp    ecx,esi
10217646:	7e e7                	jle    0x1021762f
10217648:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
1021764c:	eb 12                	jmp    0x10217660
1021764e:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
10217652:	8b f0                	mov    esi,eax
10217654:	8b f9                	mov    edi,ecx
10217656:	2b f2                	sub    esi,edx
10217658:	2b f8                	sub    edi,eax
1021765a:	3b f7                	cmp    esi,edi
1021765c:	7e 02                	jle    0x10217660
1021765e:	8b d1                	mov    edx,ecx
10217660:	85 d2                	test   edx,edx
10217662:	7e 1d                	jle    0x10217681
10217664:	8b 85 2c 01 00 00    	mov    eax,DWORD PTR [ebp+0x12c]
1021766a:	8b 0d 28 03 73 10    	mov    ecx,DWORD PTR ds:0x10730328
10217670:	4b                   	dec    ebx
10217671:	89 0c 90             	mov    DWORD PTR [eax+edx*4],ecx
10217674:	8b 44 24 14          	mov    eax,DWORD PTR [esp+0x14]
10217678:	2d 9c 05 00 00       	sub    eax,0x59c
1021767d:	89 44 24 14          	mov    DWORD PTR [esp+0x14],eax
10217681:	8b 54 24 14          	mov    edx,DWORD PTR [esp+0x14]
10217685:	8b 85 34 01 00 00    	mov    eax,DWORD PTR [ebp+0x134]
1021768b:	81 c2 9c 05 00 00    	add    edx,0x59c
10217691:	43                   	inc    ebx
10217692:	89 54 24 14          	mov    DWORD PTR [esp+0x14],edx
10217696:	8d 50 ff             	lea    edx,[eax-0x1]
10217699:	3b da                	cmp    ebx,edx
1021769b:	0f 8e dc fe ff ff    	jle    0x1021757d
102176a1:	5f                   	pop    edi
102176a2:	5e                   	pop    esi
102176a3:	5d                   	pop    ebp
102176a4:	5b                   	pop    ebx
102176a5:	81 c4 dc 00 00 00    	add    esp,0xdc
102176ab:	c3                   	ret
