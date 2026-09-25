
/mnt/data/m27_work/msi_extract/flat/mtsyc32.dll:     file format pei-i386


Disassembly of section .text:

10213080 <.text+0x212080>:
10213080:	5f                   	pop    edi
10213081:	5e                   	pop    esi
10213082:	c3                   	ret
10213083:	90                   	nop
10213084:	90                   	nop
10213085:	90                   	nop
10213086:	90                   	nop
10213087:	90                   	nop
10213088:	90                   	nop
10213089:	90                   	nop
1021308a:	90                   	nop
1021308b:	90                   	nop
1021308c:	90                   	nop
1021308d:	90                   	nop
1021308e:	90                   	nop
1021308f:	90                   	nop
10213090:	8b 4c 24 04          	mov    ecx,DWORD PTR [esp+0x4]
10213094:	8b 44 24 0c          	mov    eax,DWORD PTR [esp+0xc]
10213098:	40                   	inc    eax
10213099:	3b 81 70 01 00 00    	cmp    eax,DWORD PTR [ecx+0x170]
1021309f:	7f 23                	jg     0x102130c4
102130a1:	8b 89 6c 01 00 00    	mov    ecx,DWORD PTR [ecx+0x16c]
102130a7:	8d 04 40             	lea    eax,[eax+eax*2]
102130aa:	8d 14 81             	lea    edx,[ecx+eax*4]
102130ad:	8b 44 24 08          	mov    eax,DWORD PTR [esp+0x8]
102130b1:	8b 0a                	mov    ecx,DWORD PTR [edx]
102130b3:	89 08                	mov    DWORD PTR [eax],ecx
102130b5:	8b 4a 04             	mov    ecx,DWORD PTR [edx+0x4]
102130b8:	89 48 04             	mov    DWORD PTR [eax+0x4],ecx
102130bb:	8b 52 08             	mov    edx,DWORD PTR [edx+0x8]
102130be:	89 50 08             	mov    DWORD PTR [eax+0x8],edx
102130c1:	33 c0                	xor    eax,eax
102130c3:	c3                   	ret
102130c4:	b8 01 00 00 00       	mov    eax,0x1
102130c9:	c3                   	ret
102130ca:	90                   	nop
102130cb:	90                   	nop
102130cc:	90                   	nop
102130cd:	90                   	nop
102130ce:	90                   	nop
102130cf:	90                   	nop
102130d0:	8b 4c 24 04          	mov    ecx,DWORD PTR [esp+0x4]
102130d4:	33 c0                	xor    eax,eax
102130d6:	8a 41 04             	mov    al,BYTE PTR [ecx+0x4]
102130d9:	48                   	dec    eax
102130da:	c3                   	ret
102130db:	90                   	nop
102130dc:	90                   	nop
102130dd:	90                   	nop
102130de:	90                   	nop
102130df:	90                   	nop
102130e0:	8b 44 24 0c          	mov    eax,DWORD PTR [esp+0xc]
102130e4:	48                   	dec    eax
102130e5:	83 f8 01             	cmp    eax,0x1
102130e8:	7c 27                	jl     0x10213111
102130ea:	8b 4c 24 04          	mov    ecx,DWORD PTR [esp+0x4]
102130ee:	8d 04 40             	lea    eax,[eax+eax*2]
102130f1:	8b 91 6c 01 00 00    	mov    edx,DWORD PTR [ecx+0x16c]
102130f7:	8b 4c 24 08          	mov    ecx,DWORD PTR [esp+0x8]
102130fb:	8d 04 82             	lea    eax,[edx+eax*4]
102130fe:	8b 10                	mov    edx,DWORD PTR [eax]
10213100:	89 11                	mov    DWORD PTR [ecx],edx
10213102:	8b 50 04             	mov    edx,DWORD PTR [eax+0x4]
10213105:	89 51 04             	mov    DWORD PTR [ecx+0x4],edx
10213108:	8b 40 08             	mov    eax,DWORD PTR [eax+0x8]
1021310b:	89 41 08             	mov    DWORD PTR [ecx+0x8],eax
1021310e:	33 c0                	xor    eax,eax
10213110:	c3                   	ret
10213111:	b8 01 00 00 00       	mov    eax,0x1
10213116:	c3                   	ret
10213117:	90                   	nop
10213118:	90                   	nop
10213119:	90                   	nop
1021311a:	90                   	nop
1021311b:	90                   	nop
1021311c:	90                   	nop
1021311d:	90                   	nop
1021311e:	90                   	nop
1021311f:	90                   	nop
10213120:	8b 44 24 08          	mov    eax,DWORD PTR [esp+0x8]
10213124:	8b 4c 24 04          	mov    ecx,DWORD PTR [esp+0x4]
10213128:	c1 e0 05             	shl    eax,0x5
1021312b:	03 c1                	add    eax,ecx
1021312d:	f6 40 01 04          	test   BYTE PTR [eax+0x1],0x4
10213131:	75 09                	jne    0x1021313c
10213133:	f6 40 02 01          	test   BYTE PTR [eax+0x2],0x1
10213137:	75 03                	jne    0x1021313c
10213139:	33 c0                	xor    eax,eax
1021313b:	c3                   	ret
1021313c:	b8 01 00 00 00       	mov    eax,0x1
10213141:	c3                   	ret
10213142:	90                   	nop
10213143:	90                   	nop
10213144:	90                   	nop
10213145:	90                   	nop
10213146:	90                   	nop
10213147:	90                   	nop
10213148:	90                   	nop
10213149:	90                   	nop
1021314a:	90                   	nop
1021314b:	90                   	nop
1021314c:	90                   	nop
1021314d:	90                   	nop
1021314e:	90                   	nop
1021314f:	90                   	nop
10213150:	81 ec 04 03 00 00    	sub    esp,0x304
10213156:	53                   	push   ebx
10213157:	55                   	push   ebp
10213158:	56                   	push   esi
10213159:	57                   	push   edi
1021315a:	b9 18 00 00 00       	mov    ecx,0x18
1021315f:	33 c0                	xor    eax,eax
10213161:	8d 7c 24 69          	lea    edi,[esp+0x69]
10213165:	c6 44 24 68 00       	mov    BYTE PTR [esp+0x68],0x0
1021316a:	f3 ab                	rep stos DWORD PTR es:[edi],eax
1021316c:	66 8b 0d 58 cf 75 10 	mov    cx,WORD PTR ds:0x1075cf58
10213173:	8b b4 24 1c 03 00 00 	mov    esi,DWORD PTR [esp+0x31c]
1021317a:	8b ac 24 18 03 00 00 	mov    ebp,DWORD PTR [esp+0x318]
10213181:	8a 15 5a cf 75 10    	mov    dl,BYTE PTR ds:0x1075cf5a
10213187:	66 ab                	stos   WORD PTR es:[edi],ax
10213189:	aa                   	stos   BYTE PTR es:[edi],al
1021318a:	a1 54 cf 75 10       	mov    eax,ds:0x1075cf54
1021318f:	66 89 8c 24 dc 00 00 	mov    WORD PTR [esp+0xdc],cx
10213196:	00 
10213197:	8b 8d 6c 01 00 00    	mov    ecx,DWORD PTR [ebp+0x16c]
1021319d:	89 84 24 d8 00 00 00 	mov    DWORD PTR [esp+0xd8],eax
102131a4:	8d 04 76             	lea    eax,[esi+esi*2]
102131a7:	88 94 24 de 00 00 00 	mov    BYTE PTR [esp+0xde],dl
102131ae:	6a 02                	push   0x2
102131b0:	56                   	push   esi
102131b1:	8b 14 81             	mov    edx,DWORD PTR [ecx+eax*4]
102131b4:	8d 3c 81             	lea    edi,[ecx+eax*4]
102131b7:	55                   	push   ebp
102131b8:	89 7c 24 24          	mov    DWORD PTR [esp+0x24],edi
102131bc:	89 54 24 2c          	mov    DWORD PTR [esp+0x2c],edx
102131c0:	e8 5b 4c 01 00       	call   0x10227e20
102131c5:	6a 01                	push   0x1
102131c7:	56                   	push   esi
102131c8:	55                   	push   ebp
102131c9:	89 44 24 58          	mov    DWORD PTR [esp+0x58],eax
102131cd:	e8 4e 4c 01 00       	call   0x10227e20
102131d2:	8b d8                	mov    ebx,eax
102131d4:	83 c8 ff             	or     eax,0xffffffff
102131d7:	33 c9                	xor    ecx,ecx
102131d9:	89 44 24 3c          	mov    DWORD PTR [esp+0x3c],eax
102131dd:	8a 4f 04             	mov    cl,BYTE PTR [edi+0x4]
102131e0:	89 44 24 34          	mov    DWORD PTR [esp+0x34],eax
102131e4:	83 c4 18             	add    esp,0x18
102131e7:	33 c0                	xor    eax,eax
102131e9:	85 c9                	test   ecx,ecx
102131eb:	89 9c 24 d4 00 00 00 	mov    DWORD PTR [esp+0xd4],ebx
102131f2:	c6 44 24 3b 00       	mov    BYTE PTR [esp+0x3b],0x0
102131f7:	c6 44 24 13 00       	mov    BYTE PTR [esp+0x13],0x0
102131fc:	7e 24                	jle    0x10213222
102131fe:	8b 54 24 20          	mov    edx,DWORD PTR [esp+0x20]
10213202:	8b 7c 24 24          	mov    edi,DWORD PTR [esp+0x24]
10213206:	42                   	inc    edx
10213207:	f6 02 01             	test   BYTE PTR [edx],0x1
1021320a:	74 0a                	je     0x10213216
1021320c:	85 ff                	test   edi,edi
1021320e:	89 44 24 1c          	mov    DWORD PTR [esp+0x1c],eax
10213212:	7d 02                	jge    0x10213216
10213214:	8b f8                	mov    edi,eax
10213216:	40                   	inc    eax
10213217:	83 c2 20             	add    edx,0x20
1021321a:	3b c1                	cmp    eax,ecx
1021321c:	7c e9                	jl     0x10213207
1021321e:	89 7c 24 24          	mov    DWORD PTR [esp+0x24],edi
10213222:	8b 44 24 18          	mov    eax,DWORD PTR [esp+0x18]
10213226:	83 cf ff             	or     edi,0xffffffff
10213229:	89 7c 24 3c          	mov    DWORD PTR [esp+0x3c],edi
1021322d:	89 bc 24 cc 00 00 00 	mov    DWORD PTR [esp+0xcc],edi
10213234:	8b 50 08             	mov    edx,DWORD PTR [eax+0x8]
10213237:	85 d2                	test   edx,edx
10213239:	74 60                	je     0x1021329b
1021323b:	33 c0                	xor    eax,eax
1021323d:	85 c9                	test   ecx,ecx
1021323f:	7e 5a                	jle    0x1021329b
10213241:	8b 74 24 20          	mov    esi,DWORD PTR [esp+0x20]
10213245:	83 c6 02             	add    esi,0x2
10213248:	f6 46 ff 04          	test   BYTE PTR [esi-0x1],0x4
1021324c:	74 10                	je     0x1021325e
1021324e:	f6 c2 0f             	test   dl,0xf
10213251:	74 0b                	je     0x1021325e
10213253:	8b ac 24 18 03 00 00 	mov    ebp,DWORD PTR [esp+0x318]
1021325a:	89 44 24 3c          	mov    DWORD PTR [esp+0x3c],eax
1021325e:	f6 06 01             	test   BYTE PTR [esi],0x1
10213261:	74 0c                	je     0x1021326f
10213263:	f6 c2 f0             	test   dl,0xf0
10213266:	74 07                	je     0x1021326f
10213268:	83 ff ff             	cmp    edi,0xffffffff
1021326b:	75 02                	jne    0x1021326f
1021326d:	8b f8                	mov    edi,eax
1021326f:	40                   	inc    eax
10213270:	83 c6 20             	add    esi,0x20
10213273:	3b c1                	cmp    eax,ecx
10213275:	7c d1                	jl     0x10213248
10213277:	8b 44 24 3c          	mov    eax,DWORD PTR [esp+0x3c]
1021327b:	89 bc 24 cc 00 00 00 	mov    DWORD PTR [esp+0xcc],edi
10213282:	83 f8 ff             	cmp    eax,0xffffffff
10213285:	0f 85 df 01 00 00    	jne    0x1021346a
1021328b:	83 ff ff             	cmp    edi,0xffffffff
1021328e:	0f 85 d6 01 00 00    	jne    0x1021346a
10213294:	8b b4 24 1c 03 00 00 	mov    esi,DWORD PTR [esp+0x31c]
1021329b:	8b 8d 3c 01 00 00    	mov    ecx,DWORD PTR [ebp+0x13c]
102132a1:	8d 84 24 b0 02 00 00 	lea    eax,[esp+0x2b0]
102132a8:	8b 14 b1             	mov    edx,DWORD PTR [ecx+esi*4]
102132ab:	52                   	push   edx
102132ac:	50                   	push   eax
102132ad:	e8 3e 4b 01 00       	call   0x10227df0
102132b2:	50                   	push   eax
102132b3:	68 08 cf 75 10       	push   0x1075cf08
102132b8:	e8 63 b6 f8 ff       	call   0x1019e920
102132bd:	83 c4 10             	add    esp,0x10
102132c0:	85 c0                	test   eax,eax
102132c2:	75 08                	jne    0x102132cc
102132c4:	85 db                	test   ebx,ebx
102132c6:	0f 84 c1 00 00 00    	je     0x1021338d
102132cc:	8b 44 24 40          	mov    eax,DWORD PTR [esp+0x40]
102132d0:	85 c0                	test   eax,eax
102132d2:	0f 85 92 01 00 00    	jne    0x1021346a
102132d8:	3b b5 70 01 00 00    	cmp    esi,DWORD PTR [ebp+0x170]
102132de:	0f 8d 86 01 00 00    	jge    0x1021346a
102132e4:	8d 8c 24 00 01 00 00 	lea    ecx,[esp+0x100]
102132eb:	56                   	push   esi
102132ec:	51                   	push   ecx
102132ed:	55                   	push   ebp
102132ee:	32 db                	xor    bl,bl
102132f0:	e8 9b fd ff ff       	call   0x10213090
102132f5:	83 c4 0c             	add    esp,0xc
102132f8:	85 c0                	test   eax,eax
102132fa:	0f 85 34 01 00 00    	jne    0x10213434
10213300:	8b bc 24 1c 03 00 00 	mov    edi,DWORD PTR [esp+0x31c]
10213307:	8b b4 24 00 01 00 00 	mov    esi,DWORD PTR [esp+0x100]
1021330e:	8b 94 24 04 01 00 00 	mov    edx,DWORD PTR [esp+0x104]
10213315:	33 c0                	xor    eax,eax
10213317:	81 e2 ff 00 00 00    	and    edx,0xff
1021331d:	8d 4e 01             	lea    ecx,[esi+0x1]
10213320:	3b c2                	cmp    eax,edx
10213322:	0f 8d 0c 01 00 00    	jge    0x10213434
10213328:	f6 01 01             	test   BYTE PTR [ecx],0x1
1021332b:	75 06                	jne    0x10213333
1021332d:	40                   	inc    eax
1021332e:	83 c1 20             	add    ecx,0x20
10213331:	eb ed                	jmp    0x10213320
10213333:	8b d0                	mov    edx,eax
10213335:	c1 e2 05             	shl    edx,0x5
10213338:	8a 5c 32 0c          	mov    bl,BYTE PTR [edx+esi*1+0xc]
1021333c:	84 db                	test   bl,bl
1021333e:	0f 85 f0 00 00 00    	jne    0x10213434
10213344:	50                   	push   eax
10213345:	56                   	push   esi
10213346:	e8 d5 fd ff ff       	call   0x10213120
1021334b:	83 c4 08             	add    esp,0x8
1021334e:	85 c0                	test   eax,eax
10213350:	0f 85 de 00 00 00    	jne    0x10213434
10213356:	47                   	inc    edi
10213357:	8d 84 24 00 01 00 00 	lea    eax,[esp+0x100]
1021335e:	57                   	push   edi
1021335f:	50                   	push   eax
10213360:	55                   	push   ebp
10213361:	e8 2a fd ff ff       	call   0x10213090
10213366:	83 c4 0c             	add    esp,0xc
10213369:	85 c0                	test   eax,eax
1021336b:	74 9a                	je     0x10213307
1021336d:	8a 44 24 13          	mov    al,BYTE PTR [esp+0x13]
10213371:	84 c0                	test   al,al
10213373:	74 0c                	je     0x10213381
10213375:	8a 44 24 3b          	mov    al,BYTE PTR [esp+0x3b]
10213379:	84 c0                	test   al,al
1021337b:	0f 85 b3 00 00 00    	jne    0x10213434
10213381:	8b b4 24 1c 03 00 00 	mov    esi,DWORD PTR [esp+0x31c]
10213388:	c6 44 24 13 01       	mov    BYTE PTR [esp+0x13],0x1
1021338d:	85 f6                	test   esi,esi
1021338f:	0f 8e 37 ff ff ff    	jle    0x102132cc
10213395:	8d 8c 24 00 01 00 00 	lea    ecx,[esp+0x100]
1021339c:	56                   	push   esi
1021339d:	51                   	push   ecx
1021339e:	55                   	push   ebp
1021339f:	32 db                	xor    bl,bl
102133a1:	e8 3a fd ff ff       	call   0x102130e0
102133a6:	83 c4 0c             	add    esp,0xc
102133a9:	85 c0                	test   eax,eax
102133ab:	75 41                	jne    0x102133ee
102133ad:	8b 84 24 04 01 00 00 	mov    eax,DWORD PTR [esp+0x104]
102133b4:	8b 94 24 00 01 00 00 	mov    edx,DWORD PTR [esp+0x100]
102133bb:	25 ff 00 00 00       	and    eax,0xff
102133c0:	48                   	dec    eax
102133c1:	8b c8                	mov    ecx,eax
102133c3:	c1 e1 05             	shl    ecx,0x5
102133c6:	8d 4c 11 01          	lea    ecx,[ecx+edx*1+0x1]
102133ca:	85 c0                	test   eax,eax
102133cc:	7c 20                	jl     0x102133ee
102133ce:	f6 01 01             	test   BYTE PTR [ecx],0x1
102133d1:	75 06                	jne    0x102133d9
102133d3:	48                   	dec    eax
102133d4:	83 e9 20             	sub    ecx,0x20
102133d7:	eb f1                	jmp    0x102133ca
102133d9:	c1 e0 05             	shl    eax,0x5
102133dc:	8a 5c 10 0c          	mov    bl,BYTE PTR [eax+edx*1+0xc]
102133e0:	84 db                	test   bl,bl
102133e2:	7d 0a                	jge    0x102133ee
102133e4:	c6 44 24 3b 01       	mov    BYTE PTR [esp+0x3b],0x1
102133e9:	e9 ea fe ff ff       	jmp    0x102132d8
102133ee:	8b 54 24 18          	mov    edx,DWORD PTR [esp+0x18]
102133f2:	33 c0                	xor    eax,eax
102133f4:	8a 4a 04             	mov    cl,BYTE PTR [edx+0x4]
102133f7:	84 c9                	test   cl,cl
102133f9:	0f 86 82 01 00 00    	jbe    0x10213581
102133ff:	8b 4c 24 20          	mov    ecx,DWORD PTR [esp+0x20]
10213403:	8d 79 0c             	lea    edi,[ecx+0xc]
10213406:	8b ca                	mov    ecx,edx
10213408:	8b f7                	mov    esi,edi
1021340a:	84 db                	test   bl,bl
1021340c:	7f 02                	jg     0x10213410
1021340e:	b3 01                	mov    bl,0x1
10213410:	f6 46 f5 01          	test   BYTE PTR [esi-0xb],0x1
10213414:	74 0c                	je     0x10213422
10213416:	fe c3                	inc    bl
10213418:	83 f8 ff             	cmp    eax,0xffffffff
1021341b:	88 1e                	mov    BYTE PTR [esi],bl
1021341d:	74 03                	je     0x10213422
1021341f:	c6               	mov    BYTE PTR [edi],0x5
