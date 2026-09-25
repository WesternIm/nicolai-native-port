
/mnt/data/m27_work/msi_extract/flat/mtsyc32.dll:     file format pei-i386


Disassembly of section .text:

10214f40 <.text+0x213f40>:
10214f40:	81 ec 34 02 00 00    	sub    esp,0x234
10214f46:	56                   	push   esi
10214f47:	8b b4 24 3c 02 00 00 	mov    esi,DWORD PTR [esp+0x23c]
10214f4e:	8b 06                	mov    eax,DWORD PTR [esi]
10214f50:	8b 48 7c             	mov    ecx,DWORD PTR [eax+0x7c]
10214f53:	8b 86 34 01 00 00    	mov    eax,DWORD PTR [esi+0x134]
10214f59:	85 c0                	test   eax,eax
10214f5b:	89 4c 24 34          	mov    DWORD PTR [esp+0x34],ecx
10214f5f:	0f 84 55 09 00 00    	je     0x102158ba
10214f65:	8b 86 04 3d 00 00    	mov    eax,DWORD PTR [esi+0x3d04]
10214f6b:	83 ec 08             	sub    esp,0x8
10214f6e:	d9 80 84 00 00 00    	fld    DWORD PTR [eax+0x84]
10214f74:	d8 0d c0 6d 33 10    	fmul   DWORD PTR ds:0x10336dc0
10214f7a:	dd 5c 24 44          	fstp   QWORD PTR [esp+0x44]
10214f7e:	d9 80 84 00 00 00    	fld    DWORD PTR [eax+0x84]
10214f84:	dd 1c 24             	fstp   QWORD PTR [esp]
10214f87:	e8 b4 55 00 00       	call   0x1021a540
10214f8c:	dc 6c 24 44          	fsubr  QWORD PTR [esp+0x44]
10214f90:	83 c4 08             	add    esp,0x8
10214f93:	e8 46 39 02 00       	call   0x102388de
10214f98:	8b 8e 34 01 00 00    	mov    ecx,DWORD PTR [esi+0x134]
10214f9e:	89 44 24 08          	mov    DWORD PTR [esp+0x8],eax
10214fa2:	b8 01 00 00 00       	mov    eax,0x1
10214fa7:	3b c8                	cmp    ecx,eax
10214fa9:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
10214fad:	0f 8c 07 09 00 00    	jl     0x102158ba
10214fb3:	53                   	push   ebx
10214fb4:	55                   	push   ebp
10214fb5:	57                   	push   edi
10214fb6:	8b 7c 24 1c          	mov    edi,DWORD PTR [esp+0x1c]
10214fba:	8b 1d 1c a2 23 10    	mov    ebx,DWORD PTR ds:0x1023a21c
10214fc0:	89 7c 24 30          	mov    DWORD PTR [esp+0x30],edi
10214fc4:	c7 44 24 18 00 00 00 	mov    DWORD PTR [esp+0x18],0x0
10214fcb:	00 
10214fcc:	8b 96 2c 01 00 00    	mov    edx,DWORD PTR [esi+0x12c]
10214fd2:	8a 0c ba             	mov    cl,BYTE PTR [edx+edi*4]
10214fd5:	8d 04 ba             	lea    eax,[edx+edi*4]
10214fd8:	84 c9                	test   cl,cl
10214fda:	74 33                	je     0x1021500f
10214fdc:	bd 28 03 73 10       	mov    ebp,0x10730328
10214fe1:	8a 10                	mov    dl,BYTE PTR [eax]
10214fe3:	8a ca                	mov    cl,dl
10214fe5:	3a 55 00             	cmp    dl,BYTE PTR [ebp+0x0]
10214fe8:	75 1c                	jne    0x10215006
10214fea:	84 c9                	test   cl,cl
10214fec:	74 14                	je     0x10215002
10214fee:	8a 50 01             	mov    dl,BYTE PTR [eax+0x1]
10214ff1:	8a ca                	mov    cl,dl
10214ff3:	3a 55 01             	cmp    dl,BYTE PTR [ebp+0x1]
10214ff6:	75 0e                	jne    0x10215006
10214ff8:	83 c0 02             	add    eax,0x2
10214ffb:	83 c5 02             	add    ebp,0x2
10214ffe:	84 c9                	test   cl,cl
10215000:	75 df                	jne    0x10214fe1
10215002:	33 c0                	xor    eax,eax
10215004:	eb 05                	jmp    0x1021500b
10215006:	1b c0                	sbb    eax,eax
10215008:	83 d8 ff             	sbb    eax,0xffffffff
1021500b:	85 c0                	test   eax,eax
1021500d:	75 31                	jne    0x10215040
1021500f:	8b 86 3c 01 00 00    	mov    eax,DWORD PTR [esi+0x13c]
10215015:	6a 3e                	push   0x3e
10215017:	8b 0c b8             	mov    ecx,DWORD PTR [eax+edi*4]
1021501a:	51                   	push   ecx
1021501b:	ff d3                	call   ebx
1021501d:	83 c4 08             	add    esp,0x8
10215020:	85 c0                	test   eax,eax
10215022:	75 15                	jne    0x10215039
10215024:	8b 96 3c 01 00 00    	mov    edx,DWORD PTR [esi+0x13c]
1021502a:	6a 3c                	push   0x3c
1021502c:	8b 04 ba             	mov    eax,DWORD PTR [edx+edi*4]
1021502f:	50                   	push   eax
10215030:	ff d3                	call   ebx
10215032:	83 c4 08             	add    esp,0x8
10215035:	85 c0                	test   eax,eax
10215037:	74 04                	je     0x1021503d
10215039:	ff 44 24 18          	inc    DWORD PTR [esp+0x18]
1021503d:	47                   	inc    edi
1021503e:	eb 8c                	jmp    0x10214fcc
10215040:	8b 8e 3c 01 00 00    	mov    ecx,DWORD PTR [esi+0x13c]
10215046:	6a 3e                	push   0x3e
10215048:	89 7c 24 34          	mov    DWORD PTR [esp+0x34],edi
1021504c:	8b 14 b9             	mov    edx,DWORD PTR [ecx+edi*4]
1021504f:	52                   	push   edx
10215050:	ff d3                	call   ebx
10215052:	83 c4 08             	add    esp,0x8
10215055:	85 c0                	test   eax,eax
10215057:	75 15                	jne    0x1021506e
10215059:	8b 86 3c 01 00 00    	mov    eax,DWORD PTR [esi+0x13c]
1021505f:	6a 3c                	push   0x3c
10215061:	8b 0c b8             	mov    ecx,DWORD PTR [eax+edi*4]
10215064:	51                   	push   ecx
10215065:	ff d3                	call   ebx
10215067:	83 c4 08             	add    esp,0x8
1021506a:	85 c0                	test   eax,eax
1021506c:	74 04                	je     0x10215072
1021506e:	ff 44 24 18          	inc    DWORD PTR [esp+0x18]
10215072:	8b 44 24 1c          	mov    eax,DWORD PTR [esp+0x1c]
10215076:	8b 2d 1c a2 23 10    	mov    ebp,DWORD PTR ds:0x1023a21c
1021507c:	3b c7                	cmp    eax,edi
1021507e:	c7 44 24 10 00 00 00 	mov    DWORD PTR [esp+0x10],0x0
10215085:	00 
10215086:	8b d8                	mov    ebx,eax
10215088:	7f 2e                	jg     0x102150b8
1021508a:	8b 96 3c 01 00 00    	mov    edx,DWORD PTR [esi+0x13c]
10215090:	6a 3c                	push   0x3c
10215092:	8b 04 9a             	mov    eax,DWORD PTR [edx+ebx*4]
10215095:	50                   	push   eax
10215096:	ff d5                	call   ebp
10215098:	83 c4 08             	add    esp,0x8
1021509b:	85 c0                	test   eax,eax
1021509d:	74 08                	je     0x102150a7
1021509f:	c7 44 24 10 01 00 00 	mov    DWORD PTR [esp+0x10],0x1
102150a6:	00 
102150a7:	43                   	inc    ebx
102150a8:	3b df                	cmp    ebx,edi
102150aa:	7e de                	jle    0x1021508a
102150ac:	8b 44 24 10          	mov    eax,DWORD PTR [esp+0x10]
102150b0:	85 c0                	test   eax,eax
102150b2:	8b 44 24 1c          	mov    eax,DWORD PTR [esp+0x1c]
102150b6:	75 35                	jne    0x102150ed
102150b8:	3b c7                	cmp    eax,edi
102150ba:	8b d8                	mov    ebx,eax
102150bc:	7f 2f                	jg     0x102150ed
102150be:	8b 8e 3c 01 00 00    	mov    ecx,DWORD PTR [esi+0x13c]
102150c4:	6a 3e                	push   0x3e
102150c6:	8b 14 99             	mov    edx,DWORD PTR [ecx+ebx*4]
102150c9:	52                   	push   edx
102150ca:	ff d5                	call   ebp
102150cc:	83 c4 08             	add    esp,0x8
102150cf:	85 c0                	test   eax,eax
102150d1:	75 07                	jne    0x102150da
102150d3:	43                   	inc    ebx
102150d4:	3b df                	cmp    ebx,edi
102150d6:	7e e6                	jle    0x102150be
102150d8:	eb 0f                	jmp    0x102150e9
102150da:	68 78 cf 75 10       	push   0x1075cf78
102150df:	53                   	push   ebx
102150e0:	56                   	push   esi
102150e1:	e8 8a 17 00 00       	call   0x10216870
102150e6:	83 c4 0c             	add    esp,0xc
102150e9:	8b 44 24 1c          	mov    eax,DWORD PTR [esp+0x1c]
102150ed:	33 db                	xor    ebx,ebx
102150ef:	3b c7                	cmp    eax,edi
102150f1:	89 5c 24 24          	mov    DWORD PTR [esp+0x24],ebx
102150f5:	89 44 24 28          	mov    DWORD PTR [esp+0x28],eax
102150f9:	7f 72                	jg     0x1021516d
102150fb:	8b 86 3c 01 00 00    	mov    eax,DWORD PTR [esi+0x13c]
10215101:	8b 4c 24 28          	mov    ecx,DWORD PTR [esp+0x28]
10215105:	33 ed                	xor    ebp,ebp
10215107:	8b 3c 88             	mov    edi,DWORD PTR [eax+ecx*4]
1021510a:	8d 14 88             	lea    edx,[eax+ecx*4]
1021510d:	83 c9 ff             	or     ecx,0xffffffff
10215110:	33 c0                	xor    eax,eax
10215112:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
10215114:	f7 d1                	not    ecx
10215116:	49                   	dec    ecx
10215117:	85 c9                	test   ecx,ecx
10215119:	7e 3b                	jle    0x10215156
1021511b:	8b 12                	mov    edx,DWORD PTR [edx]
1021511d:	8b 0d 74 fe 72 10    	mov    ecx,DWORD PTR ds:0x1072fe74
10215123:	0f be 04 2a          	movsx  eax,BYTE PTR [edx+ebp*1]
10215127:	50                   	push   eax
10215128:	51                   	push   ecx
10215129:	ff 15 1c a2 23 10    	call   DWORD PTR ds:0x1023a21c
1021512f:	83 c4 08             	add    esp,0x8
10215132:	85 c0                	test   eax,eax
10215134:	74 01                	je     0x10215137
10215136:	43                   	inc    ebx
10215137:	8b 96 3c 01 00 00    	mov    edx,DWORD PTR [esi+0x13c]
1021513d:	8b 44 24 28          	mov    eax,DWORD PTR [esp+0x28]
10215141:	83 c9 ff             	or     ecx,0xffffffff
10215144:	45                   	inc    ebp
10215145:	8b 3c 82             	mov    edi,DWORD PTR [edx+eax*4]
10215148:	8d 14 82             	lea    edx,[edx+eax*4]
1021514b:	33 c0                	xor    eax,eax
1021514d:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
1021514f:	f7 d1                	not    ecx
10215151:	49                   	dec    ecx
10215152:	3b e9                	cmp    ebp,ecx
10215154:	7c c5                	jl     0x1021511b
10215156:	8b 44 24 28          	mov    eax,DWORD PTR [esp+0x28]
1021515a:	8b 4c 24 30          	mov    ecx,DWORD PTR [esp+0x30]
1021515e:	40                   	inc    eax
1021515f:	3b c1                	cmp    eax,ecx
10215161:	89 44 24 28          	mov    DWORD PTR [esp+0x28],eax
10215165:	7e 94                	jle    0x102150fb
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
102158ce:	8b 88        	mov    ecx,DWORD PTR [eax+0x3d04]
