
/mnt/data/m27_work/msi_extract/flat/mtsyc32.dll:     file format pei-i386


Disassembly of section .text:

10215160 <.text+0x214160>:
10215160:	c1 89 44 24 28 7e 94 	ror    DWORD PTR [ecx+0x7e282444],0x94
10215167:	33 ed                	xor    ebp,ebp
10215169:	8b f9                	mov    edi,ecx
1021516b:	eb 02                	jmp    0x1021516f
1021516d:	33 ed                	xor    ebp,ebp
1021516f:	8b 8e 30 01 00 00    	mov    ecx,DWORD PTR [esi+0x130]
10215175:	33 d2                	xor    edx,edx
10215177:	89 6c 24 34          	mov    DWORD PTR [esp+0x34],ebp
1021517b:	89 6c 24 3c          	mov    DWORD PTR [esp+0x3c],ebp
1021517f:	8a 04 39             	mov    al,BYTE PTR [ecx+edi*1]
10215182:	8b 7c 24 1c          	mov    edi,DWORD PTR [esp+0x1c]
10215186:	3c 03                	cmp    al,0x3
10215188:	8b 44 24 30          	mov    eax,DWORD PTR [esp+0x30]
1021518c:	0f 94 c2             	sete   dl
1021518f:	3b f8                	cmp    edi,eax
10215191:	89 54 24 38          	mov    DWORD PTR [esp+0x38],edx
10215195:	89 6c 24 44          	mov    DWORD PTR [esp+0x44],ebp
10215199:	89 7c 24 28          	mov    DWORD PTR [esp+0x28],edi
1021519d:	0f 8f fd 06 00 00    	jg     0x102158a0
102151a3:	8b 86 3c 01 00 00    	mov    eax,DWORD PTR [esi+0x13c]
102151a9:	6a 3c                	push   0x3c
102151ab:	8b 0c b8             	mov    ecx,DWORD PTR [eax+edi*4]
102151ae:	51                   	push   ecx
102151af:	e8 ac 99 f8 ff       	call   0x1019eb60
102151b4:	83 c4 08             	add    esp,0x8
102151b7:	89 44 24 2c          	mov    DWORD PTR [esp+0x2c],eax
102151bb:	85 c0                	test   eax,eax
102151bd:	7d 18                	jge    0x102151d7
102151bf:	8b 96 3c 01 00 00    	mov    edx,DWORD PTR [esi+0x13c]
102151c5:	6a 3e                	push   0x3e
102151c7:	8b 04 ba             	mov    eax,DWORD PTR [edx+edi*4]
102151ca:	50                   	push   eax
102151cb:	e8 90 99 f8 ff       	call   0x1019eb60
102151d0:	83 c4 08             	add    esp,0x8
102151d3:	89 44 24 2c          	mov    DWORD PTR [esp+0x2c],eax
102151d7:	8b 8c 24 48 02 00 00 	mov    ecx,DWORD PTR [esp+0x248]
102151de:	8b 44 24 28          	mov    eax,DWORD PTR [esp+0x28]
102151e2:	8b 91 3c 01 00 00    	mov    edx,DWORD PTR [ecx+0x13c]
102151e8:	83 c9 ff             	or     ecx,0xffffffff
102151eb:	8b 3c 82             	mov    edi,DWORD PTR [edx+eax*4]
102151ee:	8d 04 82             	lea    eax,[edx+eax*4]
102151f1:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
102151f5:	33 c0                	xor    eax,eax
102151f7:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
102151f9:	f7 d1                	not    ecx
102151fb:	2b f9                	sub    edi,ecx
102151fd:	8d 94 24 b4 00 00 00 	lea    edx,[esp+0xb4]
10215204:	8b c1                	mov    eax,ecx
10215206:	8b f7                	mov    esi,edi
10215208:	8b fa                	mov    edi,edx
1021520a:	c1 e9 02             	shr    ecx,0x2
1021520d:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
1021520f:	8b c8                	mov    ecx,eax
10215211:	8b 44 24 2c          	mov    eax,DWORD PTR [esp+0x2c]
10215215:	83 e1 03             	and    ecx,0x3
10215218:	85 c0                	test   eax,eax
1021521a:	f3 a4                	rep movs BYTE PTR es:[edi],BYTE PTR ds:[esi]
1021521c:	0f 8c 1c 04 00 00    	jl     0x1021563e
10215222:	8b 54 24 24          	mov    edx,DWORD PTR [esp+0x24]
10215226:	8b 8c 24 48 02 00 00 	mov    ecx,DWORD PTR [esp+0x248]
1021522d:	42                   	inc    edx
1021522e:	89 54 24 24          	mov    DWORD PTR [esp+0x24],edx
10215232:	8b 91 04 3d 00 00    	mov    edx,DWORD PTR [ecx+0x3d04]
10215238:	d9 42 60             	fld    DWORD PTR [edx+0x60]
1021523b:	e8 9e 36 02 00       	call   0x102388de
10215240:	39 44 24 18          	cmp    DWORD PTR [esp+0x18],eax
10215244:	0f 8e ed 00 00 00    	jle    0x10215337
1021524a:	8b 84 24 48 02 00 00 	mov    eax,DWORD PTR [esp+0x248]
10215251:	8b 54 24 30          	mov    edx,DWORD PTR [esp+0x30]
10215255:	8b 74 24 2c          	mov    esi,DWORD PTR [esp+0x2c]
10215259:	8b 88 30 01 00 00    	mov    ecx,DWORD PTR [eax+0x130]
1021525f:	0f be 0c 11          	movsx  ecx,BYTE PTR [ecx+edx*1]
10215263:	8d 04 cd 00 00 00 00 	lea    eax,[ecx*8+0x0]
1021526a:	2b c1                	sub    eax,ecx
1021526c:	8b 4c 24 40          	mov    ecx,DWORD PTR [esp+0x40]
10215270:	8b 91 58 12 08 00    	mov    edx,DWORD PTR [ecx+0x81258]
10215276:	8d 04 40             	lea    eax,[eax+eax*2]
10215279:	8d 7c 42 08          	lea    edi,[edx+eax*2+0x8]
1021527d:	8b 44 24 10          	mov    eax,DWORD PTR [esp+0x10]
10215281:	8b 00                	mov    eax,DWORD PTR [eax]
10215283:	80 3c 30 3e          	cmp    BYTE PTR [eax+esi*1],0x3e
10215287:	75 3e                	jne    0x102152c7
10215289:	8b 4c 24 18          	mov    ecx,DWORD PTR [esp+0x18]
1021528d:	8b 44 24 24          	mov    eax,DWORD PTR [esp+0x24]
10215291:	3b c1                	cmp    eax,ecx
10215293:	75 0b                	jne    0x102152a0
10215295:	0f be 47 04          	movsx  eax,BYTE PTR [edi+0x4]
10215299:	0f be 17             	movsx  edx,BYTE PTR [edi]
1021529c:	03 c2                	add    eax,edx
1021529e:	eb 03                	jmp    0x102152a3
102152a0:	0f be 07             	movsx  eax,BYTE PTR [edi]
102152a3:	50                   	push   eax
102152a4:	8d 44 24 54          	lea    eax,[esp+0x54]
102152a8:	68 70 cf 75 10       	push   0x1075cf70
102152ad:	50                   	push   eax
102152ae:	ff 15 54 a1 23 10    	call   DWORD PTR ds:0x1023a154
102152b4:	8d 8c 24 c0 00 00 00 	lea    ecx,[esp+0xc0]
102152bb:	56                   	push   esi
102152bc:	8d 54 24 60          	lea    edx,[esp+0x60]
102152c0:	51                   	push   ecx
102152c1:	52                   	push   edx
102152c2:	e9 72 01 00 00       	jmp    0x10215439
102152c7:	68 78 cf 75 10       	push   0x1075cf78
102152cc:	6a 02                	push   0x2
102152ce:	56                   	push   esi
102152cf:	50                   	push   eax
102152d0:	e8 cb 9a f8 ff       	call   0x1019eda0
102152d5:	83 c4 10             	add    esp,0x10
102152d8:	85 c0                	test   eax,eax
102152da:	75 0f                	jne    0x102152eb
102152dc:	8b 44 24 18          	mov    eax,DWORD PTR [esp+0x18]
102152e0:	8b 4c 24 24          	mov    ecx,DWORD PTR [esp+0x24]
102152e4:	3b c8                	cmp    ecx,eax
102152e6:	e9 cc 00 00 00       	jmp    0x102153b7
102152eb:	8b 94 24 48 02 00 00 	mov    edx,DWORD PTR [esp+0x248]
102152f2:	8b 4c 24 28          	mov    ecx,DWORD PTR [esp+0x28]
102152f6:	68 6c cf 75 10       	push   0x1075cf6c
102152fb:	6a 03                	push   0x3
102152fd:	8b 82 3c 01 00 00    	mov    eax,DWORD PTR [edx+0x13c]
10215303:	56                   	push   esi
10215304:	8b 14 88             	mov    edx,DWORD PTR [eax+ecx*4]
10215307:	52                   	push   edx
10215308:	e8 93 9a f8 ff       	call   0x1019eda0
1021530d:	83 c4 10             	add    esp,0x10
10215310:	85 c0                	test   eax,eax
10215312:	0f 84 e0 00 00 00    	je     0x102153f8
10215318:	8b 54 24 18          	mov    edx,DWORD PTR [esp+0x18]
1021531c:	8b 44 24 24          	mov    eax,DWORD PTR [esp+0x24]
10215320:	3b c2                	cmp    eax,edx
10215322:	0f 85 ee 00 00 00    	jne    0x10215416
10215328:	0f be 47 04          	movsx  eax,BYTE PTR [edi+0x4]
1021532c:	0f be 4f 01          	movsx  ecx,BYTE PTR [edi+0x1]
10215330:	03 c1                	add    eax,ecx
10215332:	e9 e3 00 00 00       	jmp    0x1021541a
10215337:	8b 94 24 48 02 00 00 	mov    edx,DWORD PTR [esp+0x248]
1021533e:	8b 4c 24 30          	mov    ecx,DWORD PTR [esp+0x30]
10215342:	8b 74 24 2c          	mov    esi,DWORD PTR [esp+0x2c]
10215346:	8b 82 30 01 00 00    	mov    eax,DWORD PTR [edx+0x130]
1021534c:	0f be 0c 08          	movsx  ecx,BYTE PTR [eax+ecx*1]
10215350:	8d 04 cd 00 00 00 00 	lea    eax,[ecx*8+0x0]
10215357:	2b c1                	sub    eax,ecx
10215359:	8d 14 40             	lea    edx,[eax+eax*2]
1021535c:	8b 44 24 40          	mov    eax,DWORD PTR [esp+0x40]
10215360:	8b 88 58 12 08 00    	mov    ecx,DWORD PTR [eax+0x81258]
10215366:	8d 7c 51 03          	lea    edi,[ecx+edx*2+0x3]
1021536a:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
1021536e:	8b 02                	mov    eax,DWORD PTR [edx]
10215370:	80 3c 30 3e          	cmp    BYTE PTR [eax+esi*1],0x3e
10215374:	75 22                	jne    0x10215398
10215376:	8b 44 24 18          	mov    eax,DWORD PTR [esp+0x18]
1021537a:	8b 4c 24 24          	mov    ecx,DWORD PTR [esp+0x24]
1021537e:	3b c8                	cmp    ecx,eax
10215380:	75 0e                	jne    0x10215390
10215382:	0f be 47 04          	movsx  eax,BYTE PTR [edi+0x4]
10215386:	0f be 0f             	movsx  ecx,BYTE PTR [edi]
10215389:	03 c1                	add    eax,ecx
1021538b:	e9 8a 00 00 00       	jmp    0x1021541a
10215390:	0f be 07             	movsx  eax,BYTE PTR [edi]
10215393:	e9 82 00 00 00       	jmp    0x1021541a
10215398:	68 78 cf 75 10       	push   0x1075cf78
1021539d:	6a 02                	push   0x2
1021539f:	56                   	push   esi
102153a0:	50                   	push   eax
102153a1:	e8 fa 99 f8 ff       	call   0x1019eda0
102153a6:	83 c4 10             	add    esp,0x10
102153a9:	85 c0                	test   eax,eax
102153ab:	75 1e                	jne    0x102153cb
102153ad:	8b 54 24 18          	mov    edx,DWORD PTR [esp+0x18]
102153b1:	8b 44 24 24          	mov    eax,DWORD PTR [esp+0x24]
102153b5:	3b c2                	cmp    eax,edx
102153b7:	75 0c                	jne    0x102153c5
102153b9:	0f be 47 04          	movsx  eax,BYTE PTR [edi+0x4]
102153bd:	0f be 4f 03          	movsx  ecx,BYTE PTR [edi+0x3]
102153c1:	03 c1                	add    eax,ecx
102153c3:	eb 55                	jmp    0x1021541a
102153c5:	0f be 47 03          	movsx  eax,BYTE PTR [edi+0x3]
102153c9:	eb 4f                	jmp    0x1021541a
102153cb:	8b 94 24 48 02 00 00 	mov    edx,DWORD PTR [esp+0x248]
102153d2:	8b 4c 24 28          	mov    ecx,DWORD PTR [esp+0x28]
102153d6:	68 6c cf 75 10       	push   0x1075cf6c
102153db:	6a 03                	push   0x3
102153dd:	8b 82 3c 01 00 00    	mov    eax,DWORD PTR [edx+0x13c]
102153e3:	56                   	push   esi
102153e4:	8b 14 88             	mov    edx,DWORD PTR [eax+ecx*4]
102153e7:	52                   	push   edx
102153e8:	e8 b3 99 f8 ff       	call   0x1019eda0
102153ed:	83 c4 10             	add    esp,0x10
102153f0:	85 c0                	test   eax,eax
102153f2:	0f 85 20 ff ff ff    	jne    0x10215318
102153f8:	8b 44 24 18          	mov    eax,DWORD PTR [esp+0x18]
102153fc:	8b 4c 24 24          	mov    ecx,DWORD PTR [esp+0x24]
10215400:	3b c8                	cmp    ecx,eax
10215402:	75 0c                	jne    0x10215410
10215404:	0f be 47 04          	movsx  eax,BYTE PTR [edi+0x4]
10215408:	0f be 4f 02          	movsx  ecx,BYTE PTR [edi+0x2]
1021540c:	03 c1                	add    eax,ecx
1021540e:	eb 0a                	jmp    0x1021541a
10215410:	0f be 47 02          	movsx  eax,BYTE PTR [edi+0x2]
10215414:	eb 04                	jmp    0x1021541a
10215416:	0f be 47 01          	movsx  eax,BYTE PTR [edi+0x1]
1021541a:	50                   	push   eax
1021541b:	8d 54 24 54          	lea    edx,[esp+0x54]
1021541f:	68 70 cf 75 10       	push   0x1075cf70
10215424:	52                   	push   edx
10215425:	ff 15 54 a1 23 10    	call   DWORD PTR ds:0x1023a154
1021542b:	8d 84 24 c0 00 00 00 	lea    eax,[esp+0xc0]
10215432:	56                   	push   esi
10215433:	8d 4c 24 60          	lea    ecx,[esp+0x60]
10215437:	50                   	push   eax
10215438:	51                   	push   ecx
10215439:	e8 62 98 f8 ff       	call   0x1019eca0
1021543e:	8b 94 24 60 02 00 00 	mov    edx,DWORD PTR [esp+0x260]
10215445:	8b 4c 24 48          	mov    ecx,DWORD PTR [esp+0x48]
10215449:	83 c4 18             	add    esp,0x18
1021544c:	8b 82 30 01 00 00    	mov    eax,DWORD PTR [edx+0x130]
10215452:	8b 92 3c 01 00 00    	mov    edx,DWORD PTR [edx+0x13c]
10215458:	0f be 0c 08          	movsx  ecx,BYTE PTR [eax+ecx*1]
1021545c:	8d 04 cd 00 00 00 00 	lea    eax,[ecx*8+0x0]
10215463:	2b c1                	sub    eax,ecx
10215465:	8b 4c 24 40          	mov    ecx,DWORD PTR [esp+0x40]
10215469:	8b 89 58 12 08 00    	mov    ecx,DWORD PTR [ecx+0x81258]
1021546f:	8d 04 40             	lea    eax,[eax+eax*2]
10215472:	8d 7c 41 0d          	lea    edi,[ecx+eax*2+0xd]
10215476:	8b 44 24 28          	mov    eax,DWORD PTR [esp+0x28]
1021547a:	8b 04 82             	mov    eax,DWORD PTR [edx+eax*4]
1021547d:	80 3c 30 3e          	cmp    BYTE PTR [eax+esi*1],0x3e
10215481:	75 53                	jne    0x102154d6
10215483:	8b 44 24 18          	mov    eax,DWORD PTR [esp+0x18]
10215487:	83 f8 01             	cmp    eax,0x1
1021548a:	7e 42                	jle    0x102154ce
1021548c:	0f be 4f 04          	movsx  ecx,BYTE PTR [edi+0x4]
10215490:	8b 54 24 24          	mov    edx,DWORD PTR [esp+0x24]
10215494:	89 4c 24 10          	mov    DWORD PTR [esp+0x10],ecx
10215498:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
1021549c:	4a                   	dec    edx
1021549d:	48                   	dec    eax
1021549e:	89 54 24 10          	mov    DWORD PTR [esp+0x10],edx
102154a2:	dc 2d 00 a3 23 10    	fsubr  QWORD PTR ds:0x1023a300
102154a8:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
102154ac:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
102154b0:	de c9                	fmulp  st(1),st
102154b2:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
102154b6:	0f be 0f             	movsx  ecx,BYTE PTR [edi]
102154b9:	de f9                	fdivp  st(1),st
102154bb:	89 4c 24 10          	mov    DWORD PTR [esp+0x10],ecx
102154bf:	dc 2d 00 a3 23 10    	fsubr  QWORD PTR ds:0x1023a300
102154c5:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
102154c9:	e9 e1 00 00 00       	jmp    0x102155af
102154ce:	0f be 07             	movsx  eax,BYTE PTR [edi]
102154d1:	e9 41 01 00 00       	jmp    0x10215617
102154d6:	68 6c cf 75 10       	push   0x1075cf6c
102154db:	6a 03                	push   0x3
102154dd:	56                   	push   esi
102154de:	50                   	push   eax
102154df:	e8 bc 98 f8 ff       	call   0x1019eda0
102154e4:	83 c4 10             	add    esp,0x10
102154e7:	85 c0                	test   eax,eax
102154e9:	75 54                	jne    0x1021553f
102154eb:	83 7c 24 18 01       	cmp    DWORD PTR [esp+0x18],0x1
102154f0:	7e 44                	jle    0x10215536
102154f2:	0f be 57 04          	movsx  edx,BYTE PTR [edi+0x4]
102154f6:	8b 44 24 24          	mov    eax,DWORD PTR [esp+0x24]
102154fa:	8b 4c 24 18          	mov    ecx,DWORD PTR [esp+0x18]
102154fe:	89 54 24 10          	mov    DWORD PTR [esp+0x10],edx
10215502:	48                   	dec    eax
10215503:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
10215507:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
1021550b:	49                   	dec    ecx
1021550c:	0f be 57 03          	movsx  edx,BYTE PTR [edi+0x3]
10215510:	dc 2d 00 a3 23 10    	fsubr  QWORD PTR ds:0x1023a300
10215516:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
1021551a:	89 4c 24 10          	mov    DWORD PTR [esp+0x10],ecx
1021551e:	de c9                	fmulp  st(1),st
10215520:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
10215524:	89 54 24 10          	mov    DWORD PTR [esp+0x10],edx
10215528:	de f9                	fdivp  st(1),st
1021552a:	dc 2d 00 a3 23 10    	fsubr  QWORD PTR ds:0x1023a300
10215530:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
10215534:	eb 79                	jmp    0x102155af
10215536:	0f be 47 03          	movsx  eax,BYTE PTR [edi+0x3]
1021553a:	e9 d8 00 00 00       	jmp    0x10215617
1021553f:	8b 84 24 48 02 00 00 	mov    eax,DWORD PTR [esp+0x248]
10215546:	8b 54 24 28          	mov    edx,DWORD PTR [esp+0x28]
1021554a:	68 78 cf 75 10       	push   0x1075cf78
1021554f:	6a 02                	push   0x2
10215551:	8b 88 3c 01 00 00    	mov    ecx,DWORD PTR [eax+0x13c]
10215557:	56                   	push   esi
10215558:	8b 04 91             	mov    eax,DWORD PTR [ecx+edx*4]
1021555b:	50                   	push   eax
1021555c:	e8 3f 98 f8 ff       	call   0x1019eda0
10215561:	83 c4 10             	add    esp,0x10
10215564:	85 c0                	test   eax,eax
10215566:	8b 44 24 18          	mov    eax,DWORD PTR [esp+0x18]
1021556a:	75 5e                	jne    0x102155ca
1021556c:	83 f8 01             	cmp    eax,0x1
1021556f:	7e 53                	jle    0x102155c4
10215571:	0f be 4f 04          	movsx  ecx,BYTE PTR [edi+0x4]
10215575:	8b 54 24 24          	mov    edx,DWORD PTR [esp+0x24]
10215579:	89 4c 24 10          	mov    DWORD PTR [esp+0x10],ecx
1021557d:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
10215581:	4a                   	dec    edx
10215582:	48                   	dec    eax
10215583:	89 54 24 10          	mov    DWORD PTR [esp+0x10],edx
10215587:	dc 2d 00 a3 23 10    	fsubr  QWORD PTR ds:0x1023a300
1021558d:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
10215591:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
10215595:	de c9                	fmulp  st(1),st
10215597:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
1021559b:	0f be 4f 02          	movsx  ecx,BYTE PTR [edi+0x2]
1021559f:	de f9                	fdivp  st(1),st
102155a1:	89 4c 24 10          	mov    DWORD PTR [esp+0x10],ecx
102155a5:	dc 2d 00 a3 23 10    	fsubr  QWORD PTR ds:0x1023a300
102155ab:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
102155af:	de c9                	fmulp  st(1),st
102155b1:	dc 0d 18 a3 23 10    	fmul   QWORD PTR ds:0x1023a318
102155b7:	dc 05 f0 a2 23 10    	fadd   QWORD PTR ds:0x1023a2f0
102155bd:	e8 1c 33 02 00       	call   0x102388de
102155c2:	eb 53                	jmp    0x10215617
102155c4:	0f be 47 02          	movsx  eax,BYTE PTR [edi+0x2]
102155c8:	eb 4d                	jmp    0x10215617
102155ca:	83 f8 01             	cmp    eax,0x1
102155cd:	7e 44                	jle    0x10215613
102155cf:	0f be 57 04          	movsx  edx,BYTE PTR [edi+0x4]
102155d3:	8b 44 24 24          	mov    eax,DWORD PTR [esp+0x24]
102155d7:	8b 4c 24 18          	mov    ecx,DWORD PTR [esp+0x18]
102155db:	89 54 24 10          	mov    DWORD PTR [esp+0x10],edx
102155df:	48                   	dec    eax
102155e0:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
102155e4:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
102155e8:	49                   	dec    ecx
102155e9:	0f be 57 01          	movsx  edx,BYTE PTR [edi+0x1]
102155ed:	dc 2d 00 a3 23 10    	fsubr  QWORD PTR ds:0x1023a300
102155f3:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
102155f7:	89 4c 24 10          	mov    DWORD PTR [esp+0x10],ecx
102155fb:	de c9                	fmulp  st(1),st
102155fd:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
10215601:	89 54 24 10          	mov    DWORD PTR [esp+0x10],edx
10215605:	de f9                	fdivp  st(1),st
10215607:	dc 2d 00 a3 23 10    	fsubr  QWORD PTR ds:0x1023a300
1021560d:	db 44 24 10          	fild   DWORD PTR [esp+0x10]
10215611:	eb 9c                	jmp    0x102155af
10215613:	0f be 47 01          	movsx  eax,BYTE PTR [edi+0x1]
10215617:	50                   	push   eax
10215618:	8d 44 24 54          	lea    eax,[esp+0x54]
1021561c:	68 64 cf 75 10       	push   0x1075cf64
10215621:	50                   	push   eax
10215622:	ff 15 54 a1 23 10    	call   DWORD PTR ds:0x1023a154
10215628:	8d 8c 24 c0 00 00 00 	lea    ecx,[esp+0xc0]
1021562f:	56                   	push   esi
10215630:	8d 54 24 60          	lea    edx,[esp+0x60]
10215634:	51                   	push   ecx
10215635:	52                   	push   edx
10215636:	e8 65 96 f8 ff       	call   0x1019eca0
1021563b:	83 c4 18             	add    esp,0x18
1021563e:	8b 74            	mov    esi,DWORD PTR [esp+0x28]
