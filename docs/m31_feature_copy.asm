
/mnt/data/m27_work/msi_extract/flat/mtsyc32.dll:     file format pei-i386


Disassembly of section .text:

10212d40 <.text+0x211d40>:
10212d40:	6c                   	ins    BYTE PTR es:[edi],dx
10212d41:	24 10                	and    al,0x10
10212d43:	8d 0c 80             	lea    ecx,[eax+eax*4]
10212d46:	8d 14 c9             	lea    edx,[ecx+ecx*8]
10212d49:	c1 e2 03             	shl    edx,0x3
10212d4c:	2b d0                	sub    edx,eax
10212d4e:	8b 86 24 01 00 00    	mov    eax,DWORD PTR [esi+0x124]
10212d54:	8a 0c 90             	mov    cl,BYTE PTR [eax+edx*4]
10212d57:	8d 04 90             	lea    eax,[eax+edx*4]
10212d5a:	fe c1                	inc    cl
10212d5c:	8a 54 24 30          	mov    dl,BYTE PTR [esp+0x30]
10212d60:	88 08                	mov    BYTE PTR [eax],cl
10212d62:	81 e1 ff 00 00 00    	and    ecx,0xff
10212d68:	f7 db                	neg    ebx
10212d6a:	8d 0c 89             	lea    ecx,[ecx+ecx*4]
10212d6d:	1a db                	sbb    bl,bl
10212d6f:	8d 44 88 04          	lea    eax,[eax+ecx*4+0x4]
10212d73:	8a 4c 24 2c          	mov    cl,BYTE PTR [esp+0x2c]
10212d77:	23 dd                	and    ebx,ebp
10212d79:	88 48 01             	mov    BYTE PTR [eax+0x1],cl
10212d7c:	8a 4c 24 34          	mov    cl,BYTE PTR [esp+0x34]
10212d80:	88 50 02             	mov    BYTE PTR [eax+0x2],dl
10212d83:	8a 54 24 24          	mov    dl,BYTE PTR [esp+0x24]
10212d87:	88 48 05             	mov    BYTE PTR [eax+0x5],cl
10212d8a:	8b 4c 24 3c          	mov    ecx,DWORD PTR [esp+0x3c]
10212d8e:	88 18                	mov    BYTE PTR [eax],bl
10212d90:	88 50 03             	mov    BYTE PTR [eax+0x3],dl
10212d93:	8b be 58 01 00 00    	mov    edi,DWORD PTR [esi+0x158]
10212d99:	81 e1 ff 00 00 00    	and    ecx,0xff
10212d9f:	80 fa 01             	cmp    dl,0x1
10212da2:	8a 1c 0f             	mov    bl,BYTE PTR [edi+ecx*1]
10212da5:	88 58 0c             	mov    BYTE PTR [eax+0xc],bl
10212da8:	8b be 5c 01 00 00    	mov    edi,DWORD PTR [esi+0x15c]
10212dae:	8b 3c 8f             	mov    edi,DWORD PTR [edi+ecx*4]
10212db1:	89 78 10             	mov    DWORD PTR [eax+0x10],edi
10212db4:	74 05                	je     0x10212dbb
10212db6:	80 fa 02             	cmp    dl,0x2
10212db9:	75 0d                	jne    0x10212dc8
10212dbb:	8b 96 60 01 00 00    	mov    edx,DWORD PTR [esi+0x160]
10212dc1:	8a 4c ca 06          	mov    cl,BYTE PTR [edx+ecx*8+0x6]
10212dc5:	88 48 04             	mov    BYTE PTR [eax+0x4],cl
10212dc8:	5f                   	pop    edi
10212dc9:	5e                   	pop    esi
10212dca:	5d                   	pop    ebp
10212dcb:	5b                   	pop    ebx
10212dcc:	83 c4 08             	add    esp,0x8
10212dcf:	c3                   	ret
10212dd0:	c0 28 21             	shr    BYTE PTR [eax],0x21
10212dd3:	10 c0                	adc    al,al
10212dd5:	28 21                	sub    BYTE PTR [ecx],ah
10212dd7:	10 c0                	adc    al,al
10212dd9:	28 21                	sub    BYTE PTR [ecx],ah
10212ddb:	10 e7                	adc    bh,ah
10212ddd:	28 21                	sub    BYTE PTR [ecx],ah
10212ddf:	10 c0                	adc    al,al
10212de1:	28 21                	sub    BYTE PTR [ecx],ah
10212de3:	10 e7                	adc    bh,ah
10212de5:	28 21                	sub    BYTE PTR [ecx],ah
10212de7:	10 c0                	adc    al,al
10212de9:	28 21                	sub    BYTE PTR [ecx],ah
10212deb:	10 05 29 21 10 56    	adc    BYTE PTR ds:0x56102129,al
10212df1:	8b 74 24 08          	mov    esi,DWORD PTR [esp+0x8]
10212df5:	57                   	push   edi
10212df6:	6a 00                	push   0x0
10212df8:	8b 06                	mov    eax,DWORD PTR [esi]
10212dfa:	56                   	push   esi
10212dfb:	6a 00                	push   0x0
10212dfd:	8b 78 7c             	mov    edi,DWORD PTR [eax+0x7c]
10212e00:	e8 7b f5 f8 ff       	call   0x101a2380
10212e05:	56                   	push   esi
10212e06:	e8 95 f4 f8 ff       	call   0x101a22a0
10212e0b:	8b 8f 64 12 08 00    	mov    ecx,DWORD PTR [edi+0x81264]
10212e11:	51                   	push   ecx
10212e12:	56                   	push   esi
10212e13:	e8 a8 ef f8 ff       	call   0x101a1dc0
10212e18:	8b 86 70 01 00 00    	mov    eax,DWORD PTR [esi+0x170]
10212e1e:	bf 01 00 00 00       	mov    edi,0x1
10212e23:	83 c4 18             	add    esp,0x18
10212e26:	3b c7                	cmp    eax,edi
10212e28:	7c 37                	jl     0x10212e61
10212e2a:	57                   	push   edi
10212e2b:	56                   	push   esi
10212e2c:	89 be 78 33 00 00    	mov    DWORD PTR [esi+0x3378],edi
10212e32:	e8 19 03 00 00       	call   0x10213150
10212e37:	56                   	push   esi
10212e38:	e8 83 49 01 00       	call   0x102277c0
10212e3d:	56                   	push   esi
10212e3e:	e8 ad 3e 01 00       	call   0x10226cf0
10212e43:	56                   	push   esi
10212e44:	57                   	push   edi
10212e45:	e8 86 ef f8 ff       	call   0x101a1dd0
10212e4a:	6a 00                	push   0x0
10212e4c:	56                   	push   esi
10212e4d:	57                   	push   edi
10212e4e:	e8 2d f5 f8 ff       	call   0x101a2380
10212e53:	8b 86 70 01 00 00    	mov    eax,DWORD PTR [esi+0x170]
10212e59:	83 c4 24             	add    esp,0x24
10212e5c:	47                   	inc    edi
10212e5d:	3b f8                	cmp    edi,eax
10212e5f:	7e c9                	jle    0x10212e2a
10212e61:	6a 01                	push   0x1
10212e63:	56                   	push   esi
10212e64:	6a 00                	push   0x0
10212e66:	e8 15 f5 f8 ff       	call   0x101a2380
10212e6b:	83 c4 0c             	add    esp,0xc
10212e6e:	5f                   	pop    edi
10212e6f:	5e                   	pop    esi
10212e70:	c3                   	ret
10212e71:	90                   	nop
10212e72:	90                   	nop
10212e73:	90                   	nop
10212e74:	90                   	nop
10212e75:	90                   	nop
10212e76:	90                   	nop
10212e77:	90                   	nop
10212e78:	90                   	nop
10212e79:	90                   	nop
10212e7a:	90                   	nop
10212e7b:	90                   	nop
10212e7c:	90                   	nop
10212e7d:	90                   	nop
10212e7e:	90                   	nop
10212e7f:	90                   	nop
10212e80:	83 ec 18             	sub    esp,0x18
10212e83:	8b 44 24 2c          	mov    eax,DWORD PTR [esp+0x2c]
10212e87:	8b 54 24 30          	mov    edx,DWORD PTR [esp+0x30]
10212e8b:	53                   	push   ebx
10212e8c:	8b 5c 24 20          	mov    ebx,DWORD PTR [esp+0x20]
10212e90:	55                   	push   ebp
10212e91:	8d 2c 40             	lea    ebp,[eax+eax*2]
10212e94:	8b 8b 6c 01 00 00    	mov    ecx,DWORD PTR [ebx+0x16c]
10212e9a:	56                   	push   esi
10212e9b:	57                   	push   edi
10212e9c:	8b fa                	mov    edi,edx
10212e9e:	c1 e5 02             	shl    ebp,0x2
10212ea1:	c1 e7 05             	shl    edi,0x5
10212ea4:	8b 34 29             	mov    esi,DWORD PTR [ecx+ebp*1]
10212ea7:	b9 08 00 00 00       	mov    ecx,0x8
10212eac:	03 f7                	add    esi,edi
10212eae:	8b 7c 24 34          	mov    edi,DWORD PTR [esp+0x34]
10212eb2:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
10212eb4:	8b 8b 6c 01 00 00    	mov    ecx,DWORD PTR [ebx+0x16c]
10212eba:	8b f8                	mov    edi,eax
10212ebc:	8b c5                	mov    eax,ebp
10212ebe:	8b 74 24 38          	mov    esi,DWORD PTR [esp+0x38]
10212ec2:	03 c1                	add    eax,ecx
10212ec4:	52                   	push   edx
10212ec5:	56                   	push   esi
10212ec6:	8b 08                	mov    ecx,DWORD PTR [eax]
10212ec8:	89 4c 24 24          	mov    DWORD PTR [esp+0x24],ecx
10212ecc:	8b 48 04             	mov    ecx,DWORD PTR [eax+0x4]
10212ecf:	89 4c 24 28          	mov    DWORD PTR [esp+0x28],ecx
10212ed3:	8d 4c 24 24          	lea    ecx,[esp+0x24]
10212ed7:	8b 40 08             	mov    eax,DWORD PTR [eax+0x8]
10212eda:	51                   	push   ecx
10212edb:	89 44 24 30          	mov    DWORD PTR [esp+0x30],eax
10212edf:	e8 ec 00 00 00       	call   0x10212fd0
10212ee4:	83 c4 0c             	add    esp,0xc
10212ee7:	85 c0                	test   eax,eax
10212ee9:	74 4f                	je     0x10212f3a
10212eeb:	8d 54 24 1c          	lea    edx,[esp+0x1c]
10212eef:	57                   	push   edi
10212ef0:	52                   	push   edx
10212ef1:	53                   	push   ebx
10212ef2:	e8 99 01 00 00       	call   0x10213090
10212ef7:	83 c4 0c             	add    esp,0xc
10212efa:	85 c0                	test   eax,eax
10212efc:	75 23                	jne    0x10212f21
10212efe:	8b 83 2c 01 00 00    	mov    eax,DWORD PTR [ebx+0x12c]
10212f04:	80 3c b8 00          	cmp    BYTE PTR [eax+edi*4],0x0
10212f08:	75 24                	jne    0x10212f2e
10212f0a:	6a ff                	push   0xffffffff
10212f0c:	8d 4c 24 20          	lea    ecx,[esp+0x20]
10212f10:	56                   	push   esi
10212f11:	51                   	push   ecx
10212f12:	47                   	inc    edi
10212f13:	e8 b8 00 00 00       	call   0x10212fd0
10212f18:	83 c4 0c             	add    esp,0xc
10212f1b:	85 c0                	test   eax,eax
10212f1d:	75 cc                	jne    0x10212eeb
10212f1f:	eb 19                	jmp    0x10212f3a
10212f21:	c6 06 20             	mov    BYTE PTR [esi],0x20
10212f24:	c6 46 01 00          	mov    BYTE PTR [esi+0x1],0x0
10212f28:	c6 46 03 00          	mov    BYTE PTR [esi+0x3],0x0
10212f2c:	eb 0c                	jmp    0x10212f3a
10212f2e:	c6 06 20             	mov    BYTE PTR [esi],0x20
10212f31:	c6 46 01 00          	mov    BYTE PTR [esi+0x1],0x0
10212f35:	c6 46 03 00          	mov    BYTE PTR [esi+0x3],0x0
10212f39:	47                   	inc    edi
10212f3a:	8b 83 6c 01 00 00    	mov    eax,DWORD PTR [ebx+0x16c]
10212f40:	8b 74 24 3c          	mov    esi,DWORD PTR [esp+0x3c]
10212f44:	03 e8                	add    ebp,eax
10212f46:	8b 55 00             	mov    edx,DWORD PTR [ebp+0x0]
10212f49:	89 54 24 10          	mov    DWORD PTR [esp+0x10],edx
10212f4d:	8b 54 24 40          	mov    edx,DWORD PTR [esp+0x40]
10212f51:	8b 45 04             	mov    eax,DWORD PTR [ebp+0x4]
10212f54:	52                   	push   edx
10212f55:	89 44 24 18          	mov    DWORD PTR [esp+0x18],eax
10212f59:	8d 44 24 14          	lea    eax,[esp+0x14]
10212f5d:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
10212f60:	8b 6c 24 34          	mov    ebp,DWORD PTR [esp+0x34]
10212f64:	55                   	push   ebp
10212f65:	50                   	push   eax
10212f66:	89 4c 24 24          	mov    DWORD PTR [esp+0x24],ecx
10212f6a:	e8 c1 00 00 00       	call   0x10213030
10212f6f:	83 c4 0c             	add    esp,0xc
10212f72:	85 c0                	test   eax,eax
10212f74:	74 48                	je     0x10212fbe
10212f76:	8d 4c 24 10          	lea    ecx,[esp+0x10]
10212f7a:	56                   	push   esi
10212f7b:	51                   	push   ecx
10212f7c:	53                   	push   ebx
10212f7d:	e8 5e 01 00 00       	call   0x102130e0
10212f82:	83 c4 0c             	add    esp,0xc
10212f85:	85 c0                	test   eax,eax
10212f87:	75 29                	jne    0x10212fb2
10212f89:	8d 54 24 10          	lea    edx,[esp+0x10]
10212f8d:	52                   	push   edx
10212f8e:	e8 3d 01 00 00       	call   0x102130d0
10212f93:	40                   	inc    eax
10212f94:	4e                   	dec    esi
10212f95:	50                   	push   eax
10212f96:	8d 44 24 18          	lea    eax,[esp+0x18]
10212f9a:	55                   	push   ebp
10212f9b:	50                   	push   eax
10212f9c:	e8 8f 00 00 00       	call   0x10213030
10212fa1:	83 c4 10             	add    esp,0x10
10212fa4:	85 c0                	test   eax,eax
10212fa6:	75 ce                	jne    0x10212f76
10212fa8:	8b c7                	mov    eax,edi
10212faa:	5f                   	pop    edi
10212fab:	5e                   	pop    esi
10212fac:	5d                   	pop    ebp
10212fad:	5b                   	pop    ebx
10212fae:	83 c4 18             	add    esp,0x18
10212fb1:	c3                   	ret
10212fb2:	c6 45 00 20          	mov    BYTE PTR [ebp+0x0],0x20
10212fb6:	c6 45 01 00          	mov    BYTE PTR [ebp+0x1],0x0
10212fba:	c6 45 03 00          	mov    BYTE PTR [ebp+0x3],0x0
10212fbe:	8b c7                	mov    eax,edi
10212fc0:	5f                   	pop    edi
10212fc1:	5e                   	pop    esi
10212fc2:	5d                   	pop    ebp
10212fc3:	5b                   	pop    ebx
10212fc4:	83 c4 18             	add    esp,0x18
10212fc7:	c3                   	ret
10212fc8:	90                   	nop
10212fc9:	90                   	nop
10212fca:	90                   	nop
10212fcb:	90                   	nop
10212fcc:	90                   	nop
10212fcd:	90                   	nop
10212fce:	90                   	nop
10212fcf:	90                   	nop
10212fd0:	8b 4c 24 04          	mov    ecx,DWORD PTR [esp+0x4]
10212fd4:	53                   	push   ebx
10212fd5:	33 c0                	xor    eax,eax
10212fd7:	56                   	push   esi
10212fd8:	8b 74 24 14          	mov    esi,DWORD PTR [esp+0x14]
10212fdc:	8a 41 04             	mov    al,BYTE PTR [ecx+0x4]
10212fdf:	57                   	push   edi
10212fe0:	46                   	inc    esi
10212fe1:	8b f8                	mov    edi,eax
10212fe3:	3b f7                	cmp    esi,edi
10212fe5:	0f 9c c0             	setl   al
10212fe8:	84 c0                	test   al,al
10212fea:	74 21                	je     0x1021300d
10212fec:	8b 19                	mov    ebx,DWORD PTR [ecx]
10212fee:	8b ce                	mov    ecx,esi
10212ff0:	c1 e1 05             	shl    ecx,0x5
10212ff3:	8d 54 19 03          	lea    edx,[ecx+ebx*1+0x3]
10212ff7:	33 c9                	xor    ecx,ecx
10212ff9:	8a 0a                	mov    cl,BYTE PTR [edx]
10212ffb:	f6 c1 0a             	test   cl,0xa
10212ffe:	74 16                	je     0x10213016
10213000:	46                   	inc    esi
10213001:	83 c2 20             	add    edx,0x20
10213004:	3b f7                	cmp    esi,edi
10213006:	0f 9c c0             	setl   al
10213009:	84 c0                	test   al,al
1021300b:	75 ea                	jne    0x10212ff7
1021300d:	5f                   	pop    edi
1021300e:	5e                   	pop    esi
1021300f:	b8 01 00 00 00       	mov    eax,0x1
10213014:	5b                   	pop    ebx
10213015:	c3                   	ret
10213016:	84 c0                	test   al,al
10213018:	74 f3                	je     0x1021300d
1021301a:	8b 7c 24 14          	mov    edi,DWORD PTR [esp+0x14]
1021301e:	b9 08 00 00 00       	mov    ecx,0x8
10213023:	c1 e6 05             	shl    esi,0x5
10213026:	03 f3                	add    esi,ebx
10213028:	33 c0                	xor    eax,eax
1021302a:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
1021302c:	5f                   	pop    edi
1021302d:	5e                   	pop    esi
1021302e:	5b                   	pop    ebx
1021302f:	c3                   	ret
10213030:	56                   	push   esi
10213031:	8b 74 24 10          	mov    esi,DWORD PTR [esp+0x10]
10213035:	4e                   	dec    esi
10213036:	57                   	push   edi
10213037:	0f 99 c1             	setns  cl
1021303a:	84 c9                	test   cl,cl
1021303c:	74 24                	je     0x10213062
1021303e:	8b 44 24 0c          	mov    eax,DWORD PTR [esp+0xc]
10213042:	8b d6                	mov    edx,esi
10213044:	c1 e2 05             	shl    edx,0x5
10213047:	8b 38                	mov    edi,DWORD PTR [eax]
10213049:	8d 54 3a 03          	lea    edx,[edx+edi*1+0x3]
1021304d:	33 c0                	xor    eax,eax
1021304f:	8a 02                	mov    al,BYTE PTR [edx]
10213051:	a8 0a                	test   al,0xa
10213053:	74 15                	je     0x1021306a
10213055:	4e                   	dec    esi
10213056:	83 ea 20             	sub    edx,0x20
10213059:	85 f6                	test   esi,esi
1021305b:	0f 9d c1             	setge  cl
1021305e:	84 c9                	test   cl,cl
10213060:	75 eb                	jne    0x1021304d
10213062:	5f                   	pop    edi
10213063:	b8 01 00 00 00       	mov    eax,0x1
10213068:	5e                   	pop    esi
10213069:	c3                   	ret
1021306a:	84 c9                	test   cl,cl
1021306c:	74 f4                	je     0x10213062
1021306e:	c1 e6 05             	shl    esi,0x5
10213071:	03 f7                	add    esi,edi
10213073:	8b 7c 24 10          	mov    edi,DWORD PTR [esp+0x10]
10213077:	b9 08 00 00 00       	mov    ecx,0x8
1021307c:	33 c0                	xor    eax,eax
1021307e:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
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
