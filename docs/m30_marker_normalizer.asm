
/mnt/data/m27_work/msi_extract/flat/mtsyc32.dll:     file format pei-i386


Disassembly of section .text:

101a1b80 <.text+0x1a0b80>:
101a1b80:	5c                   	pop    esp
101a1b81:	d0 ff                	sar    bh,1
101a1b83:	ff 8d 44 24 1c 56    	dec    DWORD PTR [ebp+0x561c2444]
101a1b89:	50                   	push   eax
101a1b8a:	6a 3c                	push   0x3c
101a1b8c:	e8 cf d0 ff ff       	call   0x1019ec60
101a1b91:	83 c4 18             	add    esp,0x18
101a1b94:	8d 7c 24 10          	lea    edi,[esp+0x10]
101a1b98:	83 c9 ff             	or     ecx,0xffffffff
101a1b9b:	33 c0                	xor    eax,eax
101a1b9d:	8b 5c 24 78          	mov    ebx,DWORD PTR [esp+0x78]
101a1ba1:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
101a1ba3:	8b ab 3c 01 00 00    	mov    ebp,DWORD PTR [ebx+0x13c]
101a1ba9:	8b 54 24 7c          	mov    edx,DWORD PTR [esp+0x7c]
101a1bad:	f7 d1                	not    ecx
101a1baf:	2b f9                	sub    edi,ecx
101a1bb1:	8b c1                	mov    eax,ecx
101a1bb3:	8b f7                	mov    esi,edi
101a1bb5:	8b 7c 95 00          	mov    edi,DWORD PTR [ebp+edx*4+0x0]
101a1bb9:	c1 e9 02             	shr    ecx,0x2
101a1bbc:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
101a1bbe:	8b c8                	mov    ecx,eax
101a1bc0:	8d 04 92             	lea    eax,[edx+edx*4]
101a1bc3:	83 e1 03             	and    ecx,0x3
101a1bc6:	f3 a4                	rep movs BYTE PTR es:[edi],BYTE PTR ds:[esi]
101a1bc8:	8d 0c c0             	lea    ecx,[eax+eax*8]
101a1bcb:	5f                   	pop    edi
101a1bcc:	c1 e1 03             	shl    ecx,0x3
101a1bcf:	2b ca                	sub    ecx,edx
101a1bd1:	8b 93 24 01 00 00    	mov    edx,DWORD PTR [ebx+0x124]
101a1bd7:	5e                   	pop    esi
101a1bd8:	5d                   	pop    ebp
101a1bd9:	c6 04 8a 00          	mov    BYTE PTR [edx+ecx*4],0x0
101a1bdd:	b0 01                	mov    al,0x1
101a1bdf:	5b                   	pop    ebx
101a1be0:	83 c4 64             	add    esp,0x64
101a1be3:	c3                   	ret
101a1be4:	5f                   	pop    edi
101a1be5:	5e                   	pop    esi
101a1be6:	5d                   	pop    ebp
101a1be7:	32 c0                	xor    al,al
101a1be9:	5b                   	pop    ebx
101a1bea:	83 c4 64             	add    esp,0x64
101a1bed:	c3                   	ret
101a1bee:	90                   	nop
101a1bef:	90                   	nop
101a1bf0:	83 ec 08             	sub    esp,0x8
101a1bf3:	53                   	push   ebx
101a1bf4:	8b 5c 24 10          	mov    ebx,DWORD PTR [esp+0x10]
101a1bf8:	8b 83 34 01 00 00    	mov    eax,DWORD PTR [ebx+0x134]
101a1bfe:	85 c0                	test   eax,eax
101a1c00:	0f 84 b1 01 00 00    	je     0x101a1db7
101a1c06:	83 f8 01             	cmp    eax,0x1
101a1c09:	c7 44 24 08 01 00 00 	mov    DWORD PTR [esp+0x8],0x1
101a1c10:	00 
101a1c11:	0f 8c a0 01 00 00    	jl     0x101a1db7
101a1c17:	55                   	push   ebp
101a1c18:	56                   	push   esi
101a1c19:	57                   	push   edi
101a1c1a:	8b 74 24 14          	mov    esi,DWORD PTR [esp+0x14]
101a1c1e:	33 ed                	xor    ebp,ebp
101a1c20:	89 74 24 10          	mov    DWORD PTR [esp+0x10],esi
101a1c24:	8b 83 2c 01 00 00    	mov    eax,DWORD PTR [ebx+0x12c]
101a1c2a:	8a 0c b0             	mov    cl,BYTE PTR [eax+esi*4]
101a1c2d:	8d 04 b0             	lea    eax,[eax+esi*4]
101a1c30:	84 c9                	test   cl,cl
101a1c32:	74 36                	je     0x101a1c6a
101a1c34:	be 28 03 73 10       	mov    esi,0x10730328
101a1c39:	8a 10                	mov    dl,BYTE PTR [eax]
101a1c3b:	8a ca                	mov    cl,dl
101a1c3d:	3a 16                	cmp    dl,BYTE PTR [esi]
101a1c3f:	75 1c                	jne    0x101a1c5d
101a1c41:	84 c9                	test   cl,cl
101a1c43:	74 14                	je     0x101a1c59
101a1c45:	8a 50 01             	mov    dl,BYTE PTR [eax+0x1]
101a1c48:	8a ca                	mov    cl,dl
101a1c4a:	3a 56 01             	cmp    dl,BYTE PTR [esi+0x1]
101a1c4d:	75 0e                	jne    0x101a1c5d
101a1c4f:	83 c0 02             	add    eax,0x2
101a1c52:	83 c6 02             	add    esi,0x2
101a1c55:	84 c9                	test   cl,cl
101a1c57:	75 e0                	jne    0x101a1c39
101a1c59:	33 c0                	xor    eax,eax
101a1c5b:	eb 05                	jmp    0x101a1c62
101a1c5d:	1b c0                	sbb    eax,eax
101a1c5f:	83 d8 ff             	sbb    eax,0xffffffff
101a1c62:	85 c0                	test   eax,eax
101a1c64:	75 3c                	jne    0x101a1ca2
101a1c66:	8b 74 24 10          	mov    esi,DWORD PTR [esp+0x10]
101a1c6a:	8b 83 3c 01 00 00    	mov    eax,DWORD PTR [ebx+0x13c]
101a1c70:	8b 3d 1c a2 23 10    	mov    edi,DWORD PTR ds:0x1023a21c
101a1c76:	6a 3e                	push   0x3e
101a1c78:	8b 0c b0             	mov    ecx,DWORD PTR [eax+esi*4]
101a1c7b:	51                   	push   ecx
101a1c7c:	ff d7                	call   edi
101a1c7e:	83 c4 08             	add    esp,0x8
101a1c81:	85 c0                	test   eax,eax
101a1c83:	75 15                	jne    0x101a1c9a
101a1c85:	8b 93 3c 01 00 00    	mov    edx,DWORD PTR [ebx+0x13c]
101a1c8b:	6a 3c                	push   0x3c
101a1c8d:	8b 04 b2             	mov    eax,DWORD PTR [edx+esi*4]
101a1c90:	50                   	push   eax
101a1c91:	ff d7                	call   edi
101a1c93:	83 c4 08             	add    esp,0x8
101a1c96:	85 c0                	test   eax,eax
101a1c98:	74 01                	je     0x101a1c9b
101a1c9a:	45                   	inc    ebp
101a1c9b:	46                   	inc    esi
101a1c9c:	89 74 24 10          	mov    DWORD PTR [esp+0x10],esi
101a1ca0:	eb 82                	jmp    0x101a1c24
101a1ca2:	8b 8b 3c 01 00 00    	mov    ecx,DWORD PTR [ebx+0x13c]
101a1ca8:	8b 7c 24 10          	mov    edi,DWORD PTR [esp+0x10]
101a1cac:	8b 35 1c a2 23 10    	mov    esi,DWORD PTR ds:0x1023a21c
101a1cb2:	6a 3e                	push   0x3e
101a1cb4:	8b 14 b9             	mov    edx,DWORD PTR [ecx+edi*4]
101a1cb7:	52                   	push   edx
101a1cb8:	ff d6                	call   esi
101a1cba:	83 c4 08             	add    esp,0x8
101a1cbd:	85 c0                	test   eax,eax
101a1cbf:	75 15                	jne    0x101a1cd6
101a1cc1:	8b 83 3c 01 00 00    	mov    eax,DWORD PTR [ebx+0x13c]
101a1cc7:	6a 3c                	push   0x3c
101a1cc9:	8b 0c b8             	mov    ecx,DWORD PTR [eax+edi*4]
101a1ccc:	51                   	push   ecx
101a1ccd:	ff d6                	call   esi
101a1ccf:	83 c4 08             	add    esp,0x8
101a1cd2:	85 c0                	test   eax,eax
101a1cd4:	74 01                	je     0x101a1cd7
101a1cd6:	45                   	inc    ebp
101a1cd7:	85 ed                	test   ebp,ebp
101a1cd9:	0f 85 bc 00 00 00    	jne    0x101a1d9b
101a1cdf:	8b 44 24 14          	mov    eax,DWORD PTR [esp+0x14]
101a1ce3:	c6 44 24 1c 01       	mov    BYTE PTR [esp+0x1c],0x1
101a1ce8:	3b f8                	cmp    edi,eax
101a1cea:	8b ef                	mov    ebp,edi
101a1cec:	7c 76                	jl     0x101a1d64
101a1cee:	8a 44 24 1c          	mov    al,BYTE PTR [esp+0x1c]
101a1cf2:	84 c0                	test   al,al
101a1cf4:	0f 84 a1 00 00 00    	je     0x101a1d9b
101a1cfa:	8b 93 3c 01 00 00    	mov    edx,DWORD PTR [ebx+0x13c]
101a1d00:	83 c9 ff             	or     ecx,0xffffffff
101a1d03:	33 c0                	xor    eax,eax
101a1d05:	8b 3c aa             	mov    edi,DWORD PTR [edx+ebp*4]
101a1d08:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
101a1d0a:	f7 d1                	not    ecx
101a1d0c:	49                   	dec    ecx
101a1d0d:	8b f1                	mov    esi,ecx
101a1d0f:	4e                   	dec    esi
101a1d10:	78 41                	js     0x101a1d53
101a1d12:	8b 83 3c 01 00 00    	mov    eax,DWORD PTR [ebx+0x13c]
101a1d18:	8b 0c a8             	mov    ecx,DWORD PTR [eax+ebp*4]
101a1d1b:	a1 74 fe 72 10       	mov    eax,ds:0x1072fe74
101a1d20:	0f be 14 31          	movsx  edx,BYTE PTR [ecx+esi*1]
101a1d24:	52                   	push   edx
101a1d25:	50                   	push   eax
101a1d26:	ff 15 1c a2 23 10    	call   DWORD PTR ds:0x1023a21c
101a1d2c:	83 c4 08             	add    esp,0x8
101a1d2f:	85 c0                	test   eax,eax
101a1d31:	75 05                	jne    0x101a1d38
101a1d33:	4e                   	dec    esi
101a1d34:	79 dc                	jns    0x101a1d12
101a1d36:	eb 1b                	jmp    0x101a1d53
101a1d38:	8b 8b 3c 01 00 00    	mov    ecx,DWORD PTR [ebx+0x13c]
101a1d3e:	46                   	inc    esi
101a1d3f:	56                   	push   esi
101a1d40:	8b 14 a9             	mov    edx,DWORD PTR [ecx+ebp*4]
101a1d43:	52                   	push   edx
101a1d44:	6a 3e                	push   0x3e
101a1d46:	e8 15 cf ff ff       	call   0x1019ec60
101a1d4b:	83 c4 0c             	add    esp,0xc
101a1d4e:	c6 44 24 1c 00       	mov    BYTE PTR [esp+0x1c],0x0
101a1d53:	8b 44 24 14          	mov    eax,DWORD PTR [esp+0x14]
101a1d57:	4d                   	dec    ebp
101a1d58:	3b e8                	cmp    ebp,eax
101a1d5a:	7d 92                	jge    0x101a1cee
101a1d5c:	8a 44 24 1c          	mov    al,BYTE PTR [esp+0x1c]
101a1d60:	84 c0                	test   al,al
101a1d62:	74 37                	je     0x101a1d9b
101a1d64:	bf 74 9e 6d 10       	mov    edi,0x106d9e74
101a1d69:	83 c9 ff             	or     ecx,0xffffffff
101a1d6c:	33 c0                	xor    eax,eax
101a1d6e:	8b 93 3c 01 00 00    	mov    edx,DWORD PTR [ebx+0x13c]
101a1d74:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
101a1d76:	8b 44 24 10          	mov    eax,DWORD PTR [esp+0x10]
101a1d7a:	f7 d1                	not    ecx
101a1d7c:	2b f9                	sub    edi,ecx
101a1d7e:	8b f7                	mov    esi,edi
101a1d80:	8b 3c 82             	mov    edi,DWORD PTR [edx+eax*4]
101a1d83:	8b e9                	mov    ebp,ecx
101a1d85:	83 c9 ff             	or     ecx,0xffffffff
101a1d88:	33 c0                	xor    eax,eax
101a1d8a:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
101a1d8c:	8b cd                	mov    ecx,ebp
101a1d8e:	4f                   	dec    edi
101a1d8f:	c1 e9 02             	shr    ecx,0x2
101a1d92:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
101a1d94:	8b cd                	mov    ecx,ebp
101a1d96:	83 e1 03             	and    ecx,0x3
101a1d99:	f3 a4                	rep movs BYTE PTR es:[edi],BYTE PTR ds:[esi]
101a1d9b:	8b 4c 24 10          	mov    ecx,DWORD PTR [esp+0x10]
101a1d9f:	8d 41 01             	lea    eax,[ecx+0x1]
101a1da2:	8b 8b 34 01 00 00    	mov    ecx,DWORD PTR [ebx+0x134]
101a1da8:	3b c1                	cmp    eax,ecx
101a1daa:	89 44 24 14          	mov    DWORD PTR [esp+0x14],eax
101a1dae:	0f 8e 66 fe ff ff    	jle    0x101a1c1a
101a1db4:	5f                   	pop    edi
101a1db5:	5e                   	pop    esi
101a1db6:	5d                   	pop    ebp
101a1db7:	5b                   	pop    ebx
101a1db8:	83 c4 08             	add    esp,0x8
101a1dbb:	c3                   	ret
101a1dbc:	90                   	nop
101a1dbd:	90                   	nop
101a1dbe:	90                   	nop
101a1dbf:	90                   	nop
101a1dc0:	8b 4c 24 04          	mov    ecx,DWORD PTR [esp+0x4]
101a1dc4:	8b 44 24 08          	mov    eax,DWORD PTR [esp+0x8]
101a1dc8:	89 81 b0 37 00 00    	mov    DWORD PTR [ecx+0x37b0],eax
101a1dce:	c3                   	ret
101a1dcf:	90                   	nop
101a1dd0:	55                   	push   ebp
101a1dd1:	8b ec                	mov    ebp,esp
101a1dd3:	83 e4 f8             	and    esp,0xfffffff8
101a1dd6:	83 ec 74             	sub    esp,0x74
101a1dd9:	53                   	push   ebx
101a1dda:	8b 5d 0c             	mov    ebx,DWORD PTR [ebp+0xc]
101a1ddd:	56                   	push   esi
101a1dde:	33 c0                	xor    eax,eax
101a1de0:	8b 8b b0 37 00 00    	mov    ecx,DWORD PTR [ebx+0x37b0]
101a1de6:	33 f6                	xor    esi,esi
101a1de8:	3b c8                	cmp    ecx,eax
101a1dea:	57                   	push   edi
101a1deb:	7f 6c                	jg     0x101a1e59
101a1ded:	8b 8b 74 33 00 00    	mov    ecx,DWORD PTR [ebx+0x3374]
101a1df3:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
101a1df7:	3b c8                	cmp    ecx,eax
101a1df9:	7e 5e                	jle    0x101a1e59
101a1dfb:	8d 83 90 01 00 00    	lea    eax,[ebx+0x190]
101a1e01:	89 44 24 14          	mov    DWORD PTR [esp+0x14],eax
101a1e05:	8b 4c 24 14          	mov    ecx,DWORD PTR [esp+0x14]
101a1e09:	33 c0                	xor    eax,eax
101a1e0b:	8d 79 e4             	lea    edi,[ecx-0x1c]
101a1e0e:	83 c9 ff             	or     ecx,0xffffffff
101a1e11:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
101a1e13:	f7 d1                	not    ecx
101a1e15:	49                   	dec    ecx
101a1e16:	8b d1                	mov    edx,ecx
101a1e18:	33 c9                	xor    ecx,ecx
101a1e1a:	85 d2                	test   edx,edx
101a1e1c:	7e 18                	jle    0x101a1e36
101a1e1e:	8b 7c 24 14          	mov    edi,DWORD PTR [esp+0x14]
101a1e22:	8b 07                	mov    eax,DWORD PTR [edi]
101a1e24:	85 c0                	test   eax,eax
101a1e26:	89 83 b0 37 00 00    	mov    DWORD PTR [ebx+0x37b0],eax
101a1e2c:	7f 29                	jg     0x101a1e57
101a1e2e:	41                   	inc    ecx
101a1e2f:	83 c7 0c             	add    edi,0xc
101a1e32:	3b ca                	cmp    ecx,edx
101a1e34:	7c ec                	jl     0x101a1e22
101a1e36:	8b 44 24 10          	mov    eax,DWORD PTR [esp+0x10]
101a1e3a:	8b 54 24 14          	mov    edx,DWORD PTR [esp+0x14]
101a1e3e:	8b 8b 74 33 00 00    	mov    ecx,DWORD PTR [ebx+0x3374]
101a1e44:	40                   	inc    eax
101a1e45:	81 c2 80 00 00 00    	add    edx,0x80
101a1e4b:	3b c1                	cmp    eax,ecx
101a1e4d:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
101a1e51:	89 54 24 14          	mov    DWORD PTR [esp+0x14],edx
101a1e55:	7c ae                	jl     0x101a1e05
101a1e57:	33 c0                	xor    eax,eax
101a1e59:	8b 8b 74 33 00 00    	mov    ecx,DWORD PTR [ebx+0x3374]
101a1e5f:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
101a1e63:	3b c8                	cmp    ecx,eax
101a1e65:	0f 8e cc 02 00 00    	jle    0x101a2137
101a1e6b:	8d 93 e8 01 00 00    	lea    edx,[ebx+0x1e8]
101a1e71:	89 44 24 30          	mov    DWORD PTR [esp+0x30],eax
101a1e75:	89 54 24 38          	mov    DWORD PTR [esp+0x38],edx
101a1e79:	8d 7a 8c             	lea    edi,[edx-0x74]
101a1e7c:	83 c9 ff             	or     ecx,0xffffffff
101a1e7f:	33 c0                	xor    eax,eax
101a1e81:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
101a1e83:	f7 d1                	not    ecx
101a1e85:	49                   	dec    ecx
101a1e86:	89 44 24 24          	mov    DWORD PTR [esp+0x24],eax
101a1e8a:	3b c8                	cmp    ecx,eax
101a1e8c:	89 4c 24 3c          	mov    DWORD PTR [esp+0x3c],ecx
101a1e90:	0f 8e 75 02 00 00    	jle    0x101a210b
101a1e96:	8d 7a a8             	lea    edi,[edx-0x58]
101a1e99:	89 44 24 2c          	mov    DWORD PTR [esp+0x2c],eax
101a1e9d:	89 54 24 28          	mov    DWORD PTR [esp+0x28],edx
101a1ea1:	89 7c 24 18          	mov    DWORD PTR [esp+0x18],edi
101a1ea5:	eb 08                	jmp    0x101a1eaf
101a1ea7:	8b 7c 24 18          	mov    edi,DWORD PTR [esp+0x18]
101a1eab:	8b 54 24 28          	mov    edx,DWORD PTR [esp+0x28]
101a1eaf:	85 f6                	test   esi,esi
101a1eb1:	75 06                	jne    0x101a1eb9
101a1eb3:	8b b3 b0 37 00 00    	mov    esi,DWORD PTR [ebx+0x37b0]
101a1eb9:	8b 07                	mov    eax,DWORD PTR [edi]
101a1ebb:	83 f8 fe             	cmp    eax,0xfffffffe
101a1ebe:	75 02                	jne    0x101a1ec2
101a1ec0:	33 f6                	xor    esi,esi
101a1ec2:	8d 4c 24 50          	lea    ecx,[esp+0x50]
101a1ec6:	66 c7 44 24 40 28 00 	mov    WORD PTR [esp+0x40],0x28
101a1ecd:	89 4c 24 44          	mov    DWORD PTR [esp+0x44],ecx
101a1ed1:	8a 0a                	mov    cl,BYTE PTR [edx]
101a1ed3:	88 4c 24 52          	mov    BYTE PTR [esp+0x52],cl
101a1ed7:	8a 4a 01             	mov    cl,BYTE PTR [edx+0x1]
101a1eda:	84 c9                	test   cl,cl
101a1edc:	c7 44 24 14 00 00 00 	mov    DWORD PTR [esp+0x14],0x0
101a1ee3:	00 
101a1ee4:	88 4c 24 53          	mov    BYTE PTR [esp+0x53],cl
101a1ee8:	75 06                	jne    0x101a1ef0
101a1eea:	b1 20                	mov    cl,0x20
101a1eec:	88 4c 24 53          	mov    BYTE PTR [esp+0x53],cl
101a1ef0:	83 f8 fe             	cmp    eax,0xfffffffe
101a1ef3:	74 18                	je     0x101a1f0d
101a1ef5:	83 f8 ff             	cmp    eax,0xffffffff
101a1ef8:	75 06                	jne    0x101a1f00
101a1efa:	8b b3 b0 37 00 00    	mov    esi,DWORD PTR [ebx+0x37b0]
101a1f00:	85 f6                	test   esi,esi
101a1f02:	7d 0b                	jge    0x101a1f0f
101a1f04:	66 c7 44 24 54 00 00 	mov    WORD PTR [esp+0x54],0x0
101a1f0b:	eb 15                	jmp    0x101a1f22
101a1f0d:	33 f6                	xor    esi,esi
101a1f0f:	56                   	push   esi
101a1f10:	53                   	push   ebx
101a1f11:	e8 3a 03 00 00       	call   0x101a2250
101a1f16:	8a 4c 24 5b          	mov    cl,BYTE PTR [esp+0x5b]
101a1f1a:	83 c4 08             	add    esp,0x8
101a1f1d:	66 89 44 24 54       	mov    WORD PTR [esp+0x54],ax
101a1f22:	8b 57 04             	mov    edx,DWORD PTR [edi+0x4]
101a1f25:	8b 07                	mov    eax,DWORD PTR [edi]
101a1f27:	3b c2                	cmp    eax,edx
101a1f29:	66 c7 44 24 56 00 00 	mov    WORD PTR [esp+0x56],0x0
101a1f30:	75 24                	jne    0x101a1f56
101a1f32:	8b 47 08             	mov    eax,DWORD PTR [edi+0x8]
101a1f35:	3b d0                	cmp    edx,eax
101a1f37:	75 1d                	jne    0x101a1f56
101a1f39:	2b c6                	sub    eax,esi
101a1f3b:	8b d0                	mov    edx,eax
101a1f3d:	b8 56 55 55 55       	mov    eax,0x55555556
101a1f42:	f7 ea                	imul   edx
101a1f44:	8b c2                	mov    eax,edx
101a1f46:	c1 e8 1f             	shr    eax,0x1f
101a1f49:	03 d0                	add    edx,eax
101a1f4b:	8d 04 32             	lea    eax,[edx+esi*1]
101a1f4e:	8d 14 56             	lea    edx,[esi+edx*2]
101a1f51:	89 07                	mov    DWORD PTR [edi],eax
101a1f53:	89 57 04             	mov    DWORD PTR [edi+0x4],edx
101a1f56:	80 7c 24 52 72       	cmp    BYTE PTR [esp+0x52],0x72
101a1f5b:	75 38                	jne    0x101a1f95
101a1f5d:	80 f9 20             	cmp    cl,0x20
101a1f60:	75 33                	jne    0x101a1f95
101a1f62:	56                   	push   esi
101a1f63:	53                   	push   ebx
101a1f64:	e8 e7 02 00 00       	call   0x101a2250
101a1f69:	83 c4 08             	add    esp,0x8
101a1f6c:	85 c0                	test   eax,eax
101a1f6e:	7c 25                	jl     0x101a1f95
101a1f70:	56                   	push   esi
101a1f71:	53                   	push   ebx
101a1f72:	e8 d9 02 00 00       	call   0x101a2250
101a1f77:	8b f8                	mov    edi,eax
101a1f79:	8b 83 04 3d 00 00    	mov    eax,DWORD PTR [ebx+0x3d04]
101a1f7f:	83 c4 08             	add    esp,0x8
101a1f82:	d9 80 88 00 00 00    	fld    DWORD PTR [eax+0x88]
101a1f88:	e8 51 69 09 00       	call   0x102388de
101a1f8d:	3b f8                	cmp    edi,eax
101a1f8f:	0f 8e 1f 01 00 00    	jle    0x101a20b4
101a1f95:	8b 4c 24 30          	mov    ecx,DWORD PTR [esp+0x30]
101a1f99:	8b 54 24 2c          	mov    edx,DWORD PTR [esp+0x2c]
101a1f9d:	03 d1                	add    edx,ecx
101a1f9f:	8d 7c 24 59          	lea    edi,[esp+0x59]
101a1fa3:	c7 44 24 48 00 00 00 	mov    DWORD PTR [esp+0x48],0x0
101a1faa:	00 
101a1fab:	c7 44 24 4c 00 00 00 	mov    DWORD PTR [esp+0x4c],0x0
101a1fb2:	00 
101a1fb3:	8d 84 93 90 01 00 00 	lea    eax,[ebx+edx*4+0x190]
101a1fba:	8b 54 24 24          	mov    edx,DWORD PTR [esp+0x24]
101a1fbe:	03 ca                	add    ecx,edx
101a1fc0:	89 7c 24 34          	mov    DWORD PTR [esp+0x34],edi
101a1fc4:	8d 94 8b d4 01 00 00 	lea    edx,[ebx+ecx*4+0x1d4]
101a1fcb:	8b 4c 24 18          	mov    ecx,DWORD PTR [esp+0x18]
101a1fcf:	89 54 24 1c          	mov    DWORD PTR [esp+0x1c],edx
101a1fd3:	89 4c 24 20          	mov    DWORD PTR [esp+0x20],ecx
101a1fd7:	8b 00                	mov    eax,DWORD PTR [eax]
101a1fd9:	83 f8 fe             	cmp    eax,0xfffffffe
101a1fdc:	74 11                	je     0x101a1fef
101a1fde:	83 f8 ff             	cmp    eax,0xffffffff
101a1fe1:	74 04                	je     0x101a1fe7
101a1fe3:	8b 31                	mov    esi,DWORD PTR [ecx]
101a1fe5:	eb 0a                	jmp    0x101a1ff1
101a1fe7:	8b b3 b0 37 00 00    	mov    esi,DWORD PTR [ebx+0x37b0]
101a1fed:	eb 02                	jmp    0x101a1ff1
101a1fef:	33 f6                	xor    esi,esi
101a1ff1:	dd 44 24 48          	fld    QWORD PTR [esp+0x48]
101a1ff5:	dc 1d e8 a2 23 10    	fcomp  QWORD PTR ds:0x1023a2e8
101a1ffb:	df e0                	fnstsw ax
101a1ffd:	f6 c4 40             	test   ah,0x40
101a2000:	74 06                	je     0x101a2008
101a2002:	db 02                	fild   DWORD PTR [edx]
101a2004:	dd 5c 24 48          	fstp   QWORD PTR [esp+0x48]
101a2008:	85 f6                	test   esi,esi
101a200a:	74 28                	je     0x101a2034
101a200c:	56                   	push   esi
101a200d:	53                   	push   ebx
101a200e:	e8 fd 02 00 00       	call   0x101a2310
101a2013:	8b 83 04 3d 00 00    	mov    eax,DWORD PTR [ebx+0x3d04]
101a2019:	83 c4 08             	add    esp,0x8
101a201c:	d8 48 74             	fmul   DWORD PTR [eax+0x74]
101a201f:	dc 4c 24 48          	fmul   QWORD PTR [esp+0x48]
101a2023:	e8 b6 68 09 00       	call   0x102388de
101a2028:	8b 4c 24 1c          	mov    ecx,DWORD PTR [esp+0x1c]
101a202c:	8b d1                	mov    edx,ecx
101a202e:	89 01                	mov    DWORD PTR [ecx],eax
101a2030:	8b 4c 24 20          	mov    ecx,DWORD PTR [esp+0x20]
101a2034:	8b c2                	mov    eax,edx
101a2036:	81 38 ff 00 00 00    	cmp    DWORD PTR [eax],0xff
101a203c:	7f 04                	jg     0x101a2042
101a203e:	8b 02                	mov    eax,DWORD PTR [edx]
101a2040:	eb 05                	jmp    0x101a2047
101a2042:	b8 ff 00 00 00       	mov    eax,0xff
101a2047:	85 f6                	test   esi,esi
101a2049:	88 47 ff             	mov    BYTE PTR [edi-0x1],al
101a204c:	7f 04                	jg     0x101a2052
101a204e:	33 c0                	xor    eax,eax
101a2050:	eb 27                	jmp    0x101a2079
101a2052:	8b 93 b0 37 00 00    	mov    edx,DWORD PTR [ebx+0x37b0]
101a2058:	52                   	push   edx
101a2059:	53                   	push   ebx
101a205a:	e8 f1 01 00 00       	call   0x101a2250
101a205f:	56                   	push   esi
101a2060:	53                   	push   ebx
101a2061:	8b f8                	mov    edi,eax
101a2063:	e8 e8 01 00 00       	call   0x101a2250
101a2068:	8b 54 24 2c          	mov    edx,DWORD PTR [esp+0x2c]
101a206c:	8b 4c 24 30          	mov    ecx,DWORD PTR [esp+0x30]
101a2070:	83 c4 10             	add    esp,0x10
101a2073:	2b c7                	sub    eax,edi
101a2075:	8b 7c 24 34          	mov    edi,DWORD PTR [esp+0x34]
101a2079:	88 07                	mov    BYTE PTR [edi],al
101a207b:	66 c7 47 01 00 00    	mov    WORD PTR [edi+0x1],0x0
101a2081:	85 f6                	test   esi,esi
101a2083:	74 06                	je     0x101a208b
101a2085:	89 b3 b0 37 00 00    	mov    DWORD PTR [ebx+0x37b0],esi
101a208b:	8b 44 24 14          	mov    eax,DWORD PTR [esp+0x14]
101a208f:	83 c1 04             	add    ecx,0x4
101a2092:	40                   	inc    eax
101a2093:	83 c7 04             	add    edi,0x4
101a2096:	83 39 00             	cmp    DWORD PTR [ecx],0x0
101a2099:	89 44 24 14          	mov    DWORD PTR [esp+0x14],eax
101a209d:	89 7c 24 34          	mov    DWORD PTR [esp+0x34],edi
101a20a1:	89 4c 24 20          	mov    DWORD PTR [esp+0x20],ecx
101a20a5:	8b c1                	mov    eax,ecx
101a20a7:	74 0b                	je     0x101a20b4
101a20a9:	83 7c 24 14 03       	cmp    DWORD PTR [esp+0x14],0x3
101a20ae:	0f 8c 23 ff ff ff    	jl     0x101a1fd7
101a20b4:	66 8b 44 24 14       	mov    ax,WORD PTR [esp+0x14]
101a20b9:	8d 4c 24 50          	lea    ecx,[esp+0x50]
101a20bd:	8d 54 24 40          	lea    edx,[esp+0x40]
101a20c1:	51                   	push   ecx
101a20c2:	52                   	push   edx
101a20c3:	53                   	push   ebx
101a20c4:	66 89 44 24 5c       	mov    WORD PTR [esp+0x5c],ax
101a20c9:	e8 82 02 00 00       	call   0x101a2350
101a20ce:	8b 4c 24 34          	mov    ecx,DWORD PTR [esp+0x34]
101a20d2:	8b 44 24 30          	mov    eax,DWORD PTR [esp+0x30]
101a20d6:	8b 7c 24 38          	mov    edi,DWORD PTR [esp+0x38]
101a20da:	8b 54 24 24          	mov    edx,DWORD PTR [esp+0x24]
101a20de:	83 c1 02             	add    ecx,0x2
101a20e1:	83 c4 0c             	add    esp,0xc
101a20e4:	89 4c 24 28          	mov    DWORD PTR [esp+0x28],ecx
101a20e8:	8b 4c 24 3c          	mov    ecx,DWORD PTR [esp+0x3c]
101a20ec:	40                   	inc    eax
101a20ed:	83 c7 03             	add    edi,0x3
101a20f0:	83 c2 0c             	add    edx,0xc
101a20f3:	3b c1                	cmp    eax,ecx
101a20f5:	89 44 24 24          	mov    DWORD PTR [esp+0x24],eax
101a20f9:	89 7c 24 2c          	mov    DWORD PTR [esp+0x2c],edi
101a20fd:	89 54 24 18          	mov    DWORD PTR [esp+0x18],edx
101a2101:	0f 8c a0 fd ff ff    	jl     0x101a1ea7
101a2107:	8b 54 24 38          	mov    edx,DWORD PTR [esp+0x38]
101a210b:	8b 44 24 10          	mov    eax,DWORD PTR [esp+0x10]
101a210f:	8b 7c 24 30          	mov    edi,DWORD PTR [esp+0x30]
101a2113:	8b 8b 74 33 00 00    	mov    ecx,DWORD PTR [ebx+0x3374]
101a2119:	40                   	inc    eax
101a211a:	81 c2 80 00 00 00    	add    edx,0x80
101a2120:	83 c7 20             	add    edi,0x20
101a2123:	3b c1                	cmp    eax,ecx
101a2125:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
101a2129:	89 54 24 38          	mov    DWORD PTR [esp+0x38],edx
101a212d:	89 7c 24 30          	mov    DWORD PTR [esp+0x30],edi
101a2131:	0f 8c 42 fd ff ff    	jl     0x101a1e79
101a2137:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
101a213a:	83 c9 ff             	or     ecx,0xffffffff
101a213d:	8d 34 85 00 00 00 00 	lea    esi,[eax*4+0x0]
101a2144:	8b 83 2c 01 00 00    	mov    eax,DWORD PTR [ebx+0x12c]
101a214a:	8b fe                	mov    edi,esi
101a214c:	03 f8                	add    edi,eax
101a214e:	33 c0                	xor    eax,eax
101a2150:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
101a2152:	f7 d1                	not    ecx
101a2154:	49                   	dec    ecx
101a2155:	0f 84 e9 00 00 00    	je     0x101a2244
101a215b:	66 89 44 24 54       	mov    WORD PTR [esp+0x54],ax
101a2160:	66 89 44 24 56       	mov    WORD PTR [esp+0x56],ax
101a2165:	88 44 24 59          	mov    BYTE PTR [esp+0x59],al
101a2169:	66 89 44 24 5a       	mov    WORD PTR [esp+0x5a],ax
101a216e:	8d 54 24 50          	lea    edx,[esp+0x50]
101a2172:	8d 44 24 40          	lea    eax,[esp+0x40]
101a2176:	52                   	push   edx
101a2177:	8d 4c 24 54          	lea    ecx,[esp+0x54]
101a217b:	50                   	push   eax
101a217c:	53                   	push   ebx
101a217d:	66 c7 44 24 4c 28 00 	mov    WORD PTR [esp+0x4c],0x28
101a2184:	89 4c 24 50          	mov    DWORD PTR [esp+0x50],ecx
101a2188:	66 c7 44 24 5c 01 00 	mov    WORD PTR [esp+0x5c],0x1
101a218f:	c6 44 24 5e 23       	mov    BYTE PTR [esp+0x5e],0x23
101a2194:	c6 44 24 5f 20       	mov    BYTE PTR [esp+0x5f],0x20
101a2199:	c6 44 24 64 32       	mov    BYTE PTR [esp+0x64],0x32
101a219e:	e8 ad 01 00 00       	call   0x101a2350
101a21a3:	8b 83 2c 01 00 00    	mov    eax,DWORD PTR [ebx+0x12c]
101a21a9:	8b fe                	mov    edi,esi
101a21ab:	03 f8                	add    edi,eax
101a21ad:	83 c9 ff             	or     ecx,0xffffffff
101a21b0:	33 c0                	xor    eax,eax
101a21b2:	83 c4 0c             	add    esp,0xc
101a21b5:	f2 ae                	repnz scas al,BYTE PTR es:[edi]
101a21b7:	f7 d1                	not    ecx
101a21b9:	49                   	dec    ecx
101a21ba:	49                   	dec    ecx
101a21bb:	74 1c                	je     0x101a21d9
101a21bd:	49                   	dec    ecx
101a21be:	74 0e                	je     0x101a21ce
101a21c0:	8b 8b 04 3d 00 00    	mov    ecx,DWORD PTR [ebx+0x3d04]
101a21c6:	d9 81 80 00 00 00    	fld    DWORD PTR [ecx+0x80]
101a21cc:	eb 14                	jmp    0x101a21e2
101a21ce:	8b 93 04 3d 00 00    	mov    edx,DWORD PTR [ebx+0x3d04]
101a21d4:	d9 42 7c             	fld    DWORD PTR [edx+0x7c]
101a21d7:	eb 09                	jmp    0x101a21e2
101a21d9:	8b 83 04 3d 00 00    	mov    eax,DWORD PTR [ebx+0x3d04]
101a21df:	d9 40 78             	fld    DWORD PTR [eax+0x78]
101a21e2:	e8 f7 66 09 00       	call   0x102388de
101a21e7:	68 99 99 c9 3f       	push   0x3fc99999
101a21ec:	68 9a 99 99 99       	push   0x9999999a
101a21f1:	89 44 24 18          	mov    DWORD PTR [esp+0x18],eax
101a21f5:	e8 46 83 07 00       	call   0x1021a540
101a21fa:	dc 2d d0 f8 25 10    	fsubr  QWORD PTR ds:0x1025f8d0
101a2200:	da 4c 24 18          	fimul  DWORD PTR [esp+0x18]
101a2204:	e8 d5 66 09 00       	call   0x102388de
101a2209:	8b 74 24 18          	mov    esi,DWORD PTR [esp+0x18]
101a220d:	66 c7 44 24 48 2b 00 	mov    WORD PTR [esp+0x48],0x2b
101a2214:	03 f0                	add    esi,eax
101a2216:	89 74 24 18          	mov    DWORD PTR [esp+0x18],esi
101a221a:	db 44 24 18          	fild   DWORD PTR [esp+0x18]
101a221e:	dc 8b 08 3d 00 00    	fmul   QWORD PTR [ebx+0x3d08]
101a2224:	e8 b5 66 09 00       	call   0x102388de
101a2229:	8b 13                	mov    edx,DWORD PTR [ebx]
101a222b:	8d 4c 24 48          	lea    ecx,[esp+0x48]
101a222f:	51                   	push   ecx
101a2230:	52                   	push   edx
101a2231:	89 44 24 54          	mov    DWORD PTR [esp+0x54],eax
101a2235:	66 c7 44 24 52 00 00 	mov    WORD PTR [esp+0x52],0x0
101a223c:	e8 3f e4 e6 ff       	call   0x10010680
101a2241:	83 c4 10             	add    esp,0x10
101a2244:	5f                   	pop    edi
101a2245:	5e                   	pop    esi
101a2246:	5b                   	pop    ebx
101a2247:	8b e5                	mov    esp,ebp
101a2249:	5d                   	pop    ebp
101a224a:	c3                   	ret
101a224b:	90                   	nop
101a224c:	90                   	nop
101a224d:	90                   	nop
101a224e:	90                   	nop
101a224f:	90                   	nop
