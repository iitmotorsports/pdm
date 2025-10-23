
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    smbus.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "stm32_config_stack.h"
#include "stm32_PMBUS_stack.h"
#include "main.h"


		
		
		

		
		
		

		
		
		

		
		
		

		
		
		

FreeMarker template error (DEBUG mode; use RETHROW in production!):
The following has evaluated to null or missing:
==> BspIpDatas  [in template "smbus_c.ftl" at line 72, column 8]

----
Tip: If the failing expression is known to legally refer to something that's sometimes null or missing, either specify a default value like myOptionalVar!myDefault, or use [#if myOptionalVar??]when-present[#else]when-missing[/#if]. (These only cover the last step of the expression; to cover the whole expression, use parenthesis: (myOptionalVar.foo)!myDefault, (myOptionalVar.foo)??
----

----
FTL stack trace ("~" means nesting-related):
	- Failed at: #list BspIpDatas as SWIP  [in template "smbus_c.ftl" at line 72, column 1]
----

Java stack trace (for programmers):
----
freemarker.core.InvalidReferenceException: [... Exception message was already printed; see it above ...]
	at freemarker.core.InvalidReferenceException.getInstance(InvalidReferenceException.java:134)
	at freemarker.core.Expression.assertNonNull(Expression.java:249)
	at freemarker.core.IteratorBlock.acceptWithResult(IteratorBlock.java:104)
	at freemarker.core.IteratorBlock.accept(IteratorBlock.java:94)
	at freemarker.core.Environment.visit(Environment.java:334)
	at freemarker.core.Environment.visit(Environment.java:340)
	at freemarker.core.Environment.process(Environment.java:313)
	at freemarker.template.Template.process(Template.java:383)
	at com.st.microxplorer.codegenerator.CodeEngine.freemarkerDo(CodeEngine.java:420)
	at com.st.microxplorer.codegenerator.CodeEngine.genCode(CodeEngine.java:273)
	at com.st.microxplorer.codegenerator.CodeGenerator.generateOutputCode(CodeGenerator.java:6459)
	at com.st.microxplorer.codegenerator.CodeGenerator.generateSpecificCode(CodeGenerator.java:5281)
	at com.st.microxplorer.codegenerator.CodeGenerator.generateSpecificCodeFile(CodeGenerator.java:1862)
	at com.st.microxplorer.codegenerator.CodeGenerator.generateCodeFiles(CodeGenerator.java:2188)
	at com.st.microxplorer.codegenerator.CodeGenerator.generateDefaultConfig(CodeGenerator.java:10991)
	at com.st.microxplorer.codegenerator.CodeGenerator.generateCode(CodeGenerator.java:1593)
	at com.st.microxplorer.plugins.projectmanager.engine.ProjectBuilder.generateCode(ProjectBuilder.java:3343)
	at com.st.microxplorer.plugins.projectmanager.engine.ProjectBuilder.createCode(ProjectBuilder.java:2235)
	at com.st.microxplorer.plugins.projectmanager.engine.ProjectBuilder.createProject(ProjectBuilder.java:819)
	at com.st.microxplorer.plugins.projectmanager.engine.GenerateProjectThread.run(GenerateProjectThread.java:61)
