using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using UnityEngine;

namespace ConstellationSim {
	public class ConstellationSimMod : PartModule {
		[KSPField(isPersistant = false, guiActive = true, guiName = "Constellation Online")]
		public bool active = false;

		[KSPEvent(guiActive = true, guiName = "Open Constellation UI", active = true, guiActiveEditor = false)]
		public void openUI() {}

		public override string GetInfo() {
			return "A part capable of using the Constellation Flight Computer";
		}
	}
}
