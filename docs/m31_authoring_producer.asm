
/mnt/data/m27_work/msi_extract/flat/mtsyc32.dll:     file format pei-i386


Disassembly of section .text:

10216b80 <.text+0x215b80>:
10216b80:	51                   	push   ecx
10216b81:	ff d3                	call   ebx
10216b83:	83 c4 08             	add    esp,0x8
10216b86:	85 c0                	test   eax,eax
10216b88:	74 23                	je     0x10216bad
10216b8a:	8d 54 24 18          	lea    edx,[esp+0x18]
10216b8e:	6a 01                	push   0x1
10216b90:	2b c2                	sub    eax,edx
10216b92:	50                   	push   eax
10216b93:	8d 44 24 20          	lea    eax,[esp+0x20]
10216b97:	50                   	push   eax
10216b98:	e8 43 80 f8 ff       	call   0x1019ebe0
10216b9d:	8d 4c 24 24          	lea    ecx,[esp+0x24]
10216ba1:	6a 3c                	push   0x3c
10216ba3:	51                   	push   ecx
10216ba4:	ff d3                	call   ebx
10216ba6:	83 c4 14             	add    esp,0x14
10216ba9:	85 c0                	test   eax,eax
10216bab:	75 dd                	jne    0x10216b8a
10216bad:	8d 54 24 18          	lea    edx,[esp+0x18]
10216bb1:	6a 3e                	push   0x3e
10216bb3:	52                   	push   edx
10216bb4:	ff d3                	call   ebx
10216bb6:	83 c4 08             	add    esp,0x8
10216bb9:	85 c0                	test   eax,eax
10216bbb:	74 23                	je     0x10216be0
10216bbd:	8d 4c 24 18          	lea    ecx,[esp+0x18]
10216bc1:	6a 01                	push   0x1
10216bc3:	2b c1                	sub    eax,ecx
10216bc5:	8d 54 24 1c          	lea    edx,[esp+0x1c]
10216bc9:	50                   	push   eax
10216bca:	52                   	push   edx
10216bcb:	e8 10 80 f8 ff       	call   0x1019ebe0
10216bd0:	8d 44 24 24          	lea    eax,[esp+0x24]
10216bd4:	6a 3e                	push   0x3e
10216bd6:	50                   	push   eax
10216bd7:	ff d3                	call   ebx
10216bd9:	83 c4 14             	add    esp,0x14
10216bdc:	85 c0                	test   eax,eax
10216bde:	75 dd                	jne    0x10216bbd
10216be0:	8d 4c 24 18          	lea    ecx,[esp+0x18]
10216be4:	8d 54 24 7c          	lea    edx,[esp+0x7c]
10216be8:	51                   	push   ecx
10216be9:	68 0c 03 73 10       	push   0x1073030c
10216bee:	52                   	push   edx
10216bef:	ff 15 54 a1 23 10    	call   DWORD PTR ds:0x1023a154
10216bf5:	8b 44 24 1c          	mov    eax,DWORD PTR [esp+0x1c]
10216bf9:	83 c4 0c             	add    esp,0xc
10216bfc:	3b e8                	cmp    ebp,eax
10216bfe:	0f 8f 15 ff ff ff    	jg     0x10216b19
10216c04:	8b b4 24 e4 00 00 00 	mov    esi,DWORD PTR [esp+0xe4]
10216c0b:	8b 86 3c 01 00 00    	mov    eax,DWORD PTR [esi+0x13c]
10216c11:	6a 3c                	push   0x3c
10216c13:	8b 0c a8             	mov    ecx,DWORD PTR [eax+ebp*4]
10216c16:	51                   	push   ecx
10216c17:	ff d3                	call   ebx
10216c19:	83 c4 08             	add    esp,0x8
10216c1c:	85 c0                	test   eax,eax
10216c1e:	74 15                	je     0x10216c35
10216c20:	3b 6c 24 10          	cmp    ebp,DWORD PTR [esp+0x10]
10216c24:	7c 0f                	jl     0x10216c35
10216c26:	68 78 cf 75 10       	push   0x1075cf78
10216c2b:	55                   	push   ebp
10216c2c:	56                   	push   esi
10216c2d:	e8 3e fc ff ff       	call   0x10216870
10216c32:	83 c4 0c             	add    esp,0xc
10216c35:	8b 54 24 14          	mov    edx,DWORD PTR [esp+0x14]
10216c39:	8b bc 24 e4 00 00 00 	mov    edi,DWORD PTR [esp+0xe4]
10216c40:	8b 6c 24 14          	mov    ebp,DWORD PTR [esp+0x14]
10216c44:	42                   	inc    edx
10216c45:	89 54 24 10          	mov    DWORD PTR [esp+0x10],edx
10216c49:	8b f2                	mov    esi,edx
10216c4b:	8b 87 34 01 00 00    	mov    eax,DWORD PTR [edi+0x134]
10216c51:	45                   	inc    ebp
10216c52:	3b e8                	cmp    ebp,eax
10216c54:	89 6c 24 14          	mov    DWORD PTR [esp+0x14],ebp
10216c58:	0f 8e 59 fd ff ff    	jle    0x102169b7
10216c5e:	5b                   	pop    ebx
10216c5f:	5f                   	pop    edi
10216c60:	5e                   	pop    esi
10216c61:	5d                   	pop    ebp
10216c62:	81 c4 d0 00 00 00    	add    esp,0xd0
10216c68:	c3                   	ret
10216c69:	90                   	nop
10216c6a:	90                   	nop
10216c6b:	90                   	nop
10216c6c:	90                   	nop
10216c6d:	90                   	nop
10216c6e:	90                   	nop
10216c6f:	90                   	nop
10216c70:	81 ec dc 00 00 00    	sub    esp,0xdc
10216c76:	53                   	push   ebx
10216c77:	55                   	push   ebp
10216c78:	56                   	push   esi
10216c79:	57                   	push   edi
10216c7a:	b9 18 00 00 00       	mov    ecx,0x18
10216c7f:	33 c0                	xor    eax,eax
10216c81:	8d 7c 24 25          	lea    edi,[esp+0x25]
10216c85:	c6 44 24 24 00       	mov    BYTE PTR [esp+0x24],0x0
10216c8a:	f3 ab                	rep stos DWORD PTR es:[edi],eax
10216c8c:	8b ac 24 f0 00 00 00 	mov    ebp,DWORD PTR [esp+0xf0]
10216c93:	66 ab                	stos   WORD PTR es:[edi],ax
10216c95:	aa                   	stos   BYTE PTR es:[edi],al
10216c96:	8b 85 34 01 00 00    	mov    eax,DWORD PTR [ebp+0x134]
10216c9c:	85 c0                	test   eax,eax
10216c9e:	0f 84 fd 09 00 00    	je     0x102176a1
10216ca4:	8b 8d 28 01 00 00    	mov    ecx,DWORD PTR [ebp+0x128]
10216caa:	bb 01 00 00 00       	mov    ebx,0x1
10216caf:	89 5c 24 10          	mov    DWORD PTR [esp+0x10],ebx
10216cb3:	c6 04 08 2c          	mov    BYTE PTR [eax+ecx*1],0x2c
10216cb7:	8b 95 34 01 00 00    	mov    edx,DWORD PTR [ebp+0x134]
10216cbd:	4a                   	dec    edx
10216cbe:	3b d3                	cmp    edx,ebx
10216cc0:	0f 8c cc 06 00 00    	jl     0x10217392
10216cc6:	8b 85 3c 01 00 00    	mov    eax,DWORD PTR [ebp+0x13c]
10216ccc:	83 c9 ff             	or     ecx,0xffffffff
10216ccf:	8d 54 24 24          	lea    edx,[esp+0x24]
10216cd3:	6a 3c                	push   0x3c
10216cd5:	8b 3c 98             	mov    edi,DWORD PTR [eax+ebx*4]
10216cd8:	33 c0                	xor    eax,eax
10216cda:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
10216cdc:	f7 d1                	not    ecx
10216cde:	2b f9                	sub    edi,ecx
10216ce0:	8b c1                	mov    eax,ecx
10216ce2:	8b f7                	mov    esi,edi
10216ce4:	8b fa                	mov    edi,edx
10216ce6:	c1 e9 02             	shr    ecx,0x2
10216ce9:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
10216ceb:	8b c8                	mov    ecx,eax
10216ced:	83 e1 03             	and    ecx,0x3
10216cf0:	f3 a4                	rep movs BYTE PTR es:[edi],BYTE PTR ds:[esi]
10216cf2:	8d 4c 24 28          	lea    ecx,[esp+0x28]
10216cf6:	51                   	push   ecx
10216cf7:	ff 15 1c a2 23 10    	call   DWORD PTR ds:0x1023a21c
10216cfd:	8b 3d 1c a2 23 10    	mov    edi,DWORD PTR ds:0x1023a21c
10216d03:	83 c4 08             	add    esp,0x8
10216d06:	85 c0                	test   eax,eax
10216d08:	74 23                	je     0x10216d2d
10216d0a:	8d 54 24 24          	lea    edx,[esp+0x24]
10216d0e:	6a 01                	push   0x1
10216d10:	2b c2                	sub    eax,edx
10216d12:	50                   	push   eax
10216d13:	8d 44 24 2c          	lea    eax,[esp+0x2c]
10216d17:	50                   	push   eax
10216d18:	e8 c3 7e f8 ff       	call   0x1019ebe0
10216d1d:	8d 4c 24 30          	lea    ecx,[esp+0x30]
10216d21:	6a 3c                	push   0x3c
10216d23:	51                   	push   ecx
10216d24:	ff d7                	call   edi
10216d26:	83 c4 14             	add    esp,0x14
10216d29:	85 c0                	test   eax,eax
10216d2b:	75 dd                	jne    0x10216d0a
10216d2d:	8d 54 24 24          	lea    edx,[esp+0x24]
10216d31:	6a 3e                	push   0x3e
10216d33:	52                   	push   edx
10216d34:	ff d7                	call   edi
10216d36:	83 c4 08             	add    esp,0x8
10216d39:	85 c0                	test   eax,eax
10216d3b:	74 23                	je     0x10216d60
10216d3d:	8d 4c 24 24          	lea    ecx,[esp+0x24]
10216d41:	6a 01                	push   0x1
10216d43:	2b c1                	sub    eax,ecx
10216d45:	8d 54 24 28          	lea    edx,[esp+0x28]
10216d49:	50                   	push   eax
10216d4a:	52                   	push   edx
10216d4b:	e8 90 7e f8 ff       	call   0x1019ebe0
10216d50:	8d 44 24 30          	lea    eax,[esp+0x30]
10216d54:	6a 3e                	push   0x3e
10216d56:	50                   	push   eax
10216d57:	ff d7                	call   edi
10216d59:	83 c4 14             	add    esp,0x14
10216d5c:	85 c0                	test   eax,eax
10216d5e:	75 dd                	jne    0x10216d3d
10216d60:	8d 34 9b             	lea    esi,[ebx+ebx*4]
10216d63:	8b 8d 24 01 00 00    	mov    ecx,DWORD PTR [ebp+0x124]
10216d69:	8d 34 f6             	lea    esi,[esi+esi*8]
10216d6c:	c1 e6 03             	shl    esi,0x3
10216d6f:	2b f3                	sub    esi,ebx
10216d71:	c1 e6 02             	shl    esi,0x2
10216d74:	89 74 24 1c          	mov    DWORD PTR [esp+0x1c],esi
10216d78:	0f be 94 0e 90 05 00 	movsx  edx,BYTE PTR [esi+ecx*1+0x590]
10216d7f:	00 
10216d80:	52                   	push   edx
10216d81:	68 94 d2 75 10       	push   0x1075d294
10216d86:	ff d7                	call   edi
10216d88:	83 c4 08             	add    esp,0x8
10216d8b:	85 c0                	test   eax,eax
10216d8d:	74 5d                	je     0x10216dec
10216d8f:	8d 44 24 24          	lea    eax,[esp+0x24]
10216d93:	8d 8c 24 88 00 00 00 	lea    ecx,[esp+0x88]
10216d9a:	50                   	push   eax
10216d9b:	68 0c 03 73 10       	push   0x1073030c
10216da0:	51                   	push   ecx
10216da1:	ff 15 54 a1 23 10    	call   DWORD PTR ds:0x1023a154
10216da7:	8b 95 24 01 00 00    	mov    edx,DWORD PTR [ebp+0x124]
10216dad:	0f be 84 16 90 05 00 	movsx  eax,BYTE PTR [esi+edx*1+0x590]
10216db4:	00 
10216db5:	50                   	push   eax
10216db6:	68 d4 7a 6c 10       	push   0x106c7ad4
10216dbb:	ff d7                	call   edi
10216dbd:	83 c4 14             	add    esp,0x14
10216dc0:	85 c0                	test   eax,eax
10216dc2:	74 19                	je     0x10216ddd
10216dc4:	8d 8c 24 88 00 00 00 	lea    ecx,[esp+0x88]
10216dcb:	51                   	push   ecx
10216dcc:	68 60 d2 75 10       	push   0x1075d260
10216dd1:	e8 4a 7b f8 ff       	call   0x1019e920
10216dd6:	83 c4 08             	add    esp,0x8
10216dd9:	85 c0                	test   eax,eax
10216ddb:	75 0f                	jne    0x10216dec
10216ddd:	8b 95 28 01 00 00    	mov    edx,DWORD PTR [ebp+0x128]
10216de3:	c6 04 13 2c          	mov    BYTE PTR [ebx+edx*1],0x2c
10216de7:	e9 92 05 00 00       	jmp    0x1021737e
10216dec:	8d 44 24 24          	lea    eax,[esp+0x24]
10216df0:	8d 8c 24 88 00 00 00 	lea    ecx,[esp+0x88]
10216df7:	50                   	push   eax
10216df8:	68 0c 03 73 10       	push   0x1073030c
10216dfd:	51                   	push   ecx
10216dfe:	ff 15 54 a1 23 10    	call   DWORD PTR ds:0x1023a154
10216e04:	8d 94 24 94 00 00 00 	lea    edx,[esp+0x94]
10216e0b:	52                   	push   edx
10216e0c:	68 b4 d1 75 10       	push   0x1075d1b4
10216e11:	e8 0a 7b f8 ff       	call   0x1019e920
10216e16:	83 c4 14             	add    esp,0x14
10216e19:	85 c0                	test   eax,eax
10216e1b:	0f 85 53 05 00 00    	jne    0x10217374
10216e21:	8d 84 24 88 00 00 00 	lea    eax,[esp+0x88]
10216e28:	50                   	push   eax
10216e29:	68 14 d1 75 10       	push   0x1075d114
10216e2e:	e8 ed 7a f8 ff       	call   0x1019e920
10216e33:	83 c4 08             	add    esp,0x8
10216e36:	85 c0                	test   eax,eax
10216e38:	0f 85 36 05 00 00    	jne    0x10217374
10216e3e:	8d 8c 24 88 00 00 00 	lea    ecx,[esp+0x88]
10216e45:	51                   	push   ecx
10216e46:	68 f0 d0 75 10       	push   0x1075d0f0
10216e4b:	e8 d0 7a f8 ff       	call   0x1019e920
10216e50:	83 c4 08             	add    esp,0x8
10216e53:	85 c0                	test   eax,eax
10216e55:	74 16                	je     0x10216e6d
10216e57:	8b 95 28 01 00 00    	mov    edx,DWORD PTR [ebp+0x128]
10216e5d:	8a 4c 13 ff          	mov    cl,BYTE PTR [ebx+edx*1-0x1]
10216e61:	8d 44 13 ff          	lea    eax,[ebx+edx*1-0x1]
10216e65:	80 f9 20             	cmp    cl,0x20
10216e68:	75 03                	jne    0x10216e6d
10216e6a:	c6 00 3d             	mov    BYTE PTR [eax],0x3d
10216e6d:	8d 84 24 88 00 00 00 	lea    eax,[esp+0x88]
10216e74:	50                   	push   eax
10216e75:	68 a0 d0 75 10       	push   0x1075d0a0
10216e7a:	e8 a1 7a f8 ff       	call   0x1019e920
10216e7f:	83 c4 08             	add    esp,0x8
10216e82:	85 c0                	test   eax,eax
10216e84:	74 33                	je     0x10216eb9
10216e86:	8b 8d 28 01 00 00    	mov    ecx,DWORD PTR [ebp+0x128]
10216e8c:	8d 44 0b ff          	lea    eax,[ebx+ecx*1-0x1]
10216e90:	8a 4c 0b ff          	mov    cl,BYTE PTR [ebx+ecx*1-0x1]
10216e94:	80 f9 20             	cmp    cl,0x20
10216e97:	75 03                	jne    0x10216e9c
10216e99:	c6 00 3d             	mov    BYTE PTR [eax],0x3d
10216e9c:	8b 95 28 01 00 00    	mov    edx,DWORD PTR [ebp+0x128]
10216ea2:	8a 0c 13             	mov    cl,BYTE PTR [ebx+edx*1]
10216ea5:	8d 04 13             	lea    eax,[ebx+edx*1]
10216ea8:	80 f9 20             	cmp    cl,0x20
10216eab:	0f 85 cd 04 00 00    	jne    0x1021737e
10216eb1:	c6 00 3d             	mov    BYTE PTR [eax],0x3d
10216eb4:	e9 c5 04 00 00       	jmp    0x1021737e
10216eb9:	8b 85 24 01 00 00    	mov    eax,DWORD PTR [ebp+0x124]
10216ebf:	c7 44 24 20 01 00 00 	mov    DWORD PTR [esp+0x20],0x1
10216ec6:	00 
10216ec7:	8a 14 30             	mov    dl,BYTE PTR [eax+esi*1]
10216eca:	8d 0c 30             	lea    ecx,[eax+esi*1]
10216ecd:	80 fa 01             	cmp    dl,0x1
10216ed0:	0f 82 a8 04 00 00    	jb     0x1021737e
10216ed6:	c7 44 24 18 14 00 00 	mov    DWORD PTR [esp+0x18],0x14
10216edd:	00 
10216ede:	8b 74 24 18          	mov    esi,DWORD PTR [esp+0x18]
10216ee2:	33 d2                	xor    edx,edx
10216ee4:	8a 54 31 07          	mov    dl,BYTE PTR [ecx+esi*1+0x7]
10216ee8:	8b ca                	mov    ecx,edx
10216eea:	83 f9 0a             	cmp    ecx,0xa
10216eed:	0f 87 4e 04 00 00    	ja     0x10217341
10216ef3:	ff 24 8d ac 76 21 10 	jmp    DWORD PTR [ecx*4+0x102176ac]
10216efa:	8b 4c 24 10          	mov    ecx,DWORD PTR [esp+0x10]
10216efe:	c7 44 24 14 01 00 00 	mov    DWORD PTR [esp+0x14],0x1
10216f05:	00 
10216f06:	41                   	inc    ecx
10216f07:	8d 14 89             	lea    edx,[ecx+ecx*4]
10216f0a:	8d 3c d2             	lea    edi,[edx+edx*8]
10216f0d:	c1 e7 03             	shl    edi,0x3
10216f10:	2b f9                	sub    edi,ecx
10216f12:	c1 e7 02             	shl    edi,0x2
10216f15:	80 3c 07 01          	cmp    BYTE PTR [edi+eax*1],0x1
10216f19:	0f 82 22 04 00 00    	jb     0x10217341
10216f1f:	8b 5c 24 18          	mov    ebx,DWORD PTR [esp+0x18]
10216f23:	be 14 00 00 00       	mov    esi,0x14
10216f28:	8b 54 24 1c          	mov    edx,DWORD PTR [esp+0x1c]
10216f2c:	33 c9                	xor    ecx,ecx
10216f2e:	03 c2                	add    eax,edx
10216f30:	8a 8c 30 a3 05 00 00 	mov    cl,BYTE PTR [eax+esi*1+0x5a3]
10216f37:	83 f9 07             	cmp    ecx,0x7
10216f3a:	0f 87 ac 00 00 00    	ja     0x10216fec
10216f40:	ff 24 8d d8 76 21 10 	jmp    DWORD PTR [ecx*4+0x102176d8]
10216f47:	8a 4c 18 09          	mov    cl,BYTE PTR [eax+ebx*1+0x9]
10216f4b:	8a 94 30 a5 05 00 00 	mov    dl,BYTE PTR [eax+esi*1+0x5a5]
10216f52:	3a ca                	cmp    cl,dl
10216f54:	74 34                	je     0x10216f8a
10216f56:	80 f9 57             	cmp    cl,0x57
10216f59:	0f 85 8d 00 00 00    	jne    0x10216fec
10216f5f:	eb 29                	jmp    0x10216f8a
10216f61:	8a 94 30 a5 05 00 00 	mov    dl,BYTE PTR [eax+esi*1+0x5a5]
10216f68:	33 c9                	xor    ecx,ecx
10216f6a:	8a 4c 18 09          	mov    cl,BYTE PTR [eax+ebx*1+0x9]
10216f6e:	52                   	push   edx
10216f6f:	68 ff 00 00 00       	push   0xff
10216f74:	6a 01                	push   0x1
10216f76:	8b 94 8d 34 3a 00 00 	mov    edx,DWORD PTR [ebp+ecx*4+0x3a34]
10216f7d:	52                   	push   edx
10216f7e:	e8 ad 7b f8 ff       	call   0x1019eb30
10216f83:	83 c4 10             	add    esp,0x10
10216f86:	85 c0                	test   eax,eax
10216f88:	74 62                	je     0x10216fec
10216f8a:	8b 85 28 01 00 00    	mov    eax,DWORD PTR [ebp+0x128]
10216f90:	8b 4c 24 10          	mov    ecx,DWORD PTR [esp+0x10]
10216f94:	03 c1                	add    eax,ecx
10216f96:	80 38 20             	cmp    BYTE PTR [eax],0x20
10216f99:	75 51                	jne    0x10216fec
10216f9b:	eb 4c                	jmp    0x10216fe9
10216f9d:	8a 4c 18 09          	mov    cl,BYTE PTR [eax+ebx*1+0x9]
10216fa1:	80 f9 14             	cmp    cl,0x14
10216fa4:	72 46                	jb     0x10216fec
10216fa6:	80 f9 2d             	cmp    cl,0x2d
10216fa9:	77 41                	ja     0x10216fec
10216fab:	8a 94 30 a5 05 00 00 	mov    dl,BYTE PTR [eax+esi*1+0x5a5]
10216fb2:	81 e1 ff 00 00 00    	and    ecx,0xff
10216fb8:	52                   	push   edx
10216fb9:	68 ff 00 00 00       	push   0xff
10216fbe:	8b 84 8d 50 3a 00 00 	mov    eax,DWORD PTR [ebp+ecx*4+0x3a50]
10216fc5:	6a 01                	push   0x1
10216fc7:	50                   	push   eax
10216fc8:	e8 63 7b f8 ff       	call   0x1019eb30
10216fcd:	83 c4 10             	add    esp,0x10
10216fd0:	85 c0                	test   eax,eax
10216fd2:	74 18                	je     0x10216fec
10216fd4:	8b 8d 28 01 00 00    	mov    ecx,DWORD PTR [ebp+0x128]
10216fda:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
10216fde:	8d 04 0a             	lea    eax,[edx+ecx*1]
10216fe1:	8a 0c 0a             	mov    cl,BYTE PTR [edx+ecx*1]
10216fe4:	80 f9 20             	cmp    cl,0x20
10216fe7:	75 03                	jne    0x10216fec
10216fe9:	c6 00 5f             	mov    BYTE PTR [eax],0x5f
10216fec:	8b 85 24 01 00 00    	mov    eax,DWORD PTR [ebp+0x124]
10216ff2:	8b 4c 24 14          	mov    ecx,DWORD PTR [esp+0x14]
10216ff6:	33 d2                	xor    edx,edx
10216ff8:	41                   	inc    ecx
10216ff9:	8a 14 07             	mov    dl,BYTE PTR [edi+eax*1]
10216ffc:	83 c6 14             	add    esi,0x14
10216fff:	3b ca                	cmp    ecx,edx
10217001:	89 4c 24 14          	mov    DWORD PTR [esp+0x14],ecx
10217005:	0f 8e 1d ff ff ff    	jle    0x10216f28
1021700b:	e9 31 03 00 00       	jmp    0x10217341
10217010:	8b 4c 24 10          	mov    ecx,DWORD PTR [esp+0x10]
10217014:	c7 44 24 14 01 00 00 	mov    DWORD PTR [esp+0x14],0x1
1021701b:	00 
1021701c:	41                   	inc    ecx
1021701d:	8d 14 89             	lea    edx,[ecx+ecx*4]
10217020:	8d 1c d2             	lea    ebx,[edx+edx*8]
10217023:	c1 e3 03             	shl    ebx,0x3
10217026:	2b d9                	sub    ebx,ecx
10217028:	c1 e3 02             	shl    ebx,0x2
1021702b:	80 3c 03 01          	cmp    BYTE PTR [ebx+eax*1],0x1
1021702f:	0f 82 0c 03 00 00    	jb     0x10217341
10217035:	8b 7c 24 1c          	mov    edi,DWORD PTR [esp+0x1c]
10217039:	be 14 00 00 00       	mov    esi,0x14
1021703e:	03 c7                	add    eax,edi
10217040:	33 d2                	xor    edx,edx
10217042:	6a ff                	push   0xffffffff
10217044:	6a 02                	push   0x2
10217046:	8a 94 30 a3 05 00 00 	mov    dl,BYTE PTR [eax+esi*1+0x5a3]
1021704d:	6a 01                	push   0x1
1021704f:	52                   	push   edx
10217050:	e8 2b b6 ff ff       	call   0x10212680
10217055:	83 c4 10             	add    esp,0x10
10217058:	84 c0                	test   al,al
1021705a:	74 41                	je     0x1021709d
1021705c:	8b 8d 24 01 00 00    	mov    ecx,DWORD PTR [ebp+0x124]
10217062:	6a ff                	push   0xffffffff
10217064:	8b c7                	mov    eax,edi
10217066:	6a 0c                	push   0xc
10217068:	6a 09                	push   0x9
1021706a:	03 c1                	add    eax,ecx
1021706c:	6a 08                	push   0x8
1021706e:	33 c9                	xor    ecx,ecx
10217070:	8a 8c 30 a5 05 00 00 	mov    cl,BYTE PTR [eax+esi*1+0x5a5]
10217077:	6a 07                	push   0x7
10217079:	6a 05                	push   0x5
1021707b:	6a 01                	push   0x1
1021707d:	51                   	push   ecx
1021707e:	e8 fd b5 ff ff       	call   0x10212680
10217083:	83 c4 20             	add    esp,0x20
10217086:	84 c0                	test   al,al
10217088:	75 13                	jne    0x1021709d
1021708a:	8b 95 28 01 00 00    	mov    edx,DWORD PTR [ebp+0x128]
10217090:	8b 44 24 10          	mov    eax,DWORD PTR [esp+0x10]
10217094:	03 c2                	add    eax,edx
10217096:	80 38 20             	cmp    BYTE PTR [eax],0x20
10217099:	75 53                	jne    0x102170ee
1021709b:	eb 4e                	jmp    0x102170eb
1021709d:	8b 8d 24 01 00 00    	mov    ecx,DWORD PTR [ebp+0x124]
102170a3:	03 cf                	add    ecx,edi
102170a5:	8a 84 31 a5 05 00 00 	mov    al,BYTE PTR [ecx+esi*1+0x5a5]
102170ac:	3c 14                	cmp    al,0x14
102170ae:	72 3e                	jb     0x102170ee
102170b0:	3c 2d                	cmp    al,0x2d
102170b2:	77 3a                	ja     0x102170ee
102170b4:	50                   	push   eax
102170b5:	8b 44 24 1c          	mov    eax,DWORD PTR [esp+0x1c]
102170b9:	33 d2                	xor    edx,edx
102170bb:	68 ff 00 00 00       	push   0xff
102170c0:	8a 54 01 09          	mov    dl,BYTE PTR [ecx+eax*1+0x9]
102170c4:	6a 01                	push   0x1
102170c6:	8b 8c 95 b4 3b 00 00 	mov    ecx,DWORD PTR [ebp+edx*4+0x3bb4]
102170cd:	51                   	push   ecx
102170ce:	e8 5d 7a f8 ff       	call   0x1019eb30
102170d3:	83 c4 10             	add    esp,0x10
102170d6:	85 c0                	test   eax,eax
102170d8:	74 14                	je     0x102170ee
102170da:	8b 95 28 01 00 00    	mov    edx,DWORD PTR [ebp+0x128]
102170e0:	8b 44 24 10          	mov    eax,DWORD PTR [esp+0x10]
102170e4:	03 c2                	add    eax,edx
102170e6:	80 38 20             	cmp    BYTE PTR [eax],0x20
102170e9:	75 03                	jne    0x102170ee
102170eb:	c6 00 5f             	mov    BYTE PTR [eax],0x5f
102170ee:	8b 85 24 01 00 00    	mov    eax,DWORD PTR [ebp+0x124]
102170f4:	8b 4c 24 14          	mov    ecx,DWORD PTR [esp+0x14]
102170f8:	33 d2                	xor    edx,edx
102170fa:	41                   	inc    ecx
102170fb:	8a 14 03             	mov    dl,BYTE PTR [ebx+eax*1]
102170fe:	83 c6 14             	add    esi,0x14
10217101:	3b ca                	cmp    ecx,edx
10217103:	89 4c 24 14          	mov    DWORD PTR [esp+0x14],ecx
10217107:	0f 8e 31 ff ff ff    	jle    0x1021703e
1021710d:	e9 2f 02 00 00       	jmp    0x10217341
10217112:	8b 4c 24 10          	mov    ecx,DWORD PTR [esp+0x10]
10217116:	bb 01 00 00 00       	mov    ebx,0x1
1021711b:	41                   	inc    ecx
1021711c:	8d 14 89             	lea    edx,[ecx+ecx*4]
1021711f:	8d 3c d2             	lea    edi,[edx+edx*8]
10217122:	c1 e7 03             	shl    edi,0x3
10217125:	2b f9                	sub    edi,ecx
10217127:	c1 e7 02             	shl    edi,0x2
1021712a:	38 1c 07             	cmp    BYTE PTR [edi+eax*1],bl
1021712d:	0f 82 0e 02 00 00    	jb     0x10217341
10217133:	be 14 00 00 00       	mov    esi,0x14
10217138:	8b 4c 24 1c          	mov    ecx,DWORD PTR [esp+0x1c]
1021713c:	8b 54 24 18          	mov    edx,DWORD PTR [esp+0x18]
10217140:	03 c1                	add    eax,ecx
10217142:	33 c9                	xor    ecx,ecx
10217144:	8a 8c 30 a3 05 00 00 	mov    cl,BYTE PTR [eax+esi*1+0x5a3]
1021714b:	49                   	dec    ecx
1021714c:	83 f9 06             	cmp    ecx,0x6
1021714f:	77 76                	ja     0x102171c7
10217151:	ff 24 8d f8 76 21 10 	jmp    DWORD PTR [ecx*4+0x102176f8]
10217158:	8a 4c 10 09          	mov    cl,BYTE PTR [eax+edx*1+0x9]
1021715c:	80 f9 14             	cmp    cl,0x14
1021715f:	72 66                	jb     0x102171c7
10217161:	80 f9 2d             	cmp    cl,0x2d
10217164:	77 61                	ja     0x102171c7
10217166:	8a 94 30 a5 05 00 00 	mov    dl,BYTE PTR [eax+esi*1+0x5a5]
1021716d:	81 e1 ff 00 00 00    	and    ecx,0xff
10217173:	52                   	push   edx
10217174:	68 ff 00 00 00       	push   0xff
10217179:	8b 84 8d dc 3b 00 00 	mov    eax,DWORD PTR [ebp+ecx*4+0x3bdc]
10217180:	6a 01                	push   0x1
10217182:	50                   	push   eax
10217183:	e8 a8 79 f8 ff       	call   0x1019eb30
10217188:	83 c4 10             	add    esp,0x10
1021718b:	85 c0                	test   eax,eax
1021718d:	74 38                	je     0x102171c7
1021718f:	8b 8d 28 01 00 00    	mov    ecx,DWORD PTR [ebp+0x128]
10217195:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
10217199:	8d 04 0a             	lea    eax,[edx+ecx*1]
1021719c:	8a 0c 0a             	mov    cl,BYTE PTR [edx+ecx*1]
1021719f:	80 f9 20             	cmp    cl,0x20
102171a2:	75 23                	jne    0x102171c7
102171a4:	eb 1e                	jmp    0x102171c4
102171a6:	8a 8c 30 a5 05 00 00 	mov    cl,BYTE PTR [eax+esi*1+0x5a5]
102171ad:	3a 4c 10 09          	cmp    cl,BYTE PTR [eax+edx*1+0x9]
102171b1:	75 14                	jne    0x102171c7
102171b3:	8b 95 28 01 00 00    	mov    edx,DWORD PTR [ebp+0x128]
102171b9:	8b 44 24 10          	mov    eax,DWORD PTR [esp+0x10]
102171bd:	03 c2                	add    eax,edx
102171bf:	80 38 20             	cmp    BYTE PTR [eax],0x20
102171c2:	75 03                	jne    0x102171c7
102171c4:	c6 00 5f             	mov    BYTE PTR [eax],0x5f
102171c7:	8b 85 24 01 00 00    	mov    eax,DWORD PTR [ebp+0x124]
102171cd:	33 c9                	xor    ecx,ecx
102171cf:	43                   	inc    ebx
102171d0:	83 c6 14             	add    esi,0x14
102171d3:	8a 0c 07             	mov    cl,BYTE PTR [edi+eax*1]
102171d6:	3b d9                	cmp    ebx,ecx
102171d8:	0f 8e 5a ff ff ff    	jle    0x10217138
102171de:	e9 5e 01 00 00       	jmp    0x10217341
102171e3:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
102171e7:	be 01 00 00 00       	mov    esi,0x1
102171ec:	42                   	inc    edx
102171ed:	8d 0c 92             	lea    ecx,[edx+edx*4]
102171f0:	8d 0c c9             	lea    ecx,[ecx+ecx*8]
102171f3:	c1 e1 03             	shl    ecx,0x3
102171f6:	2b ca                	sub    ecx,edx
102171f8:	c1 e1 02             	shl    ecx,0x2
102171fb:	80 3c 01 01          	cmp    BYTE PTR [ecx+eax*1],0x1
102171ff:	0f 82 3c 01 00 00    	jb     0x10217341
10217205:	8b 54 24 1c          	mov    edx,DWORD PTR [esp+0x1c]
10217209:	bf 14 00 00 00       	mov    edi,0x14
1021720e:	03 c2                	add    eax,edx
10217210:	03 c7                	add    eax,edi
10217212:	80 b8 a3 05 00 00 06 	cmp    BYTE PTR [eax+0x5a3],0x6
10217219:	75 1d                	jne    0x10217238
1021721b:	80 b8 a5 05 00 00 3c 	cmp    BYTE PTR [eax+0x5a5],0x3c
10217222:	75 14                	jne    0x10217238
10217224:	8b 85 28 01 00 00    	mov    eax,DWORD PTR [ebp+0x128]
1021722a:	8b 5c 24 10          	mov    ebx,DWORD PTR [esp+0x10]
1021722e:	03 c3                	add    eax,ebx
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
102176ac:	fa                   	cli
102176ad:	6e                   	outs   dx,BYTE PTR ds:[esi]
102176ae:	21 10                	and    DWORD PTR [eax],edx
